/**
 * malloc.c - a C memory allocater library targeting the K70 platform
 *
 * Single threaded allocator. Stores free chunks in segregated lists.
 * Uses bitmap to protect memory space.
 * Coalesces only when needed for performance.
 */
#include <errno.h>
#include <inttypes.h>
#include <manos.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

/*
 * Define some constants for padding and aligning data.
 * Memory from malloc will be double-word aligned.
 */
#define WORD_BYTES 4
#define WORD_PAD(n) (((n) + WORD_BYTES - 1) & ~(WORD_BYTES - 1))
#define WORD_ALIGN_PTR(addr) ((char*)WORD_PAD((uintptr_t)(addr)))
#define IS_WORD_ALIGNED(addr) (!((uintptr_t)(addr) & (WORD_BYTES - 1)))
#define DWORD_BYTES 8
#define DWORD_PAD(n) (((n) + DWORD_BYTES - 1) & ~(DWORD_BYTES - 1))
#define DWORD_ALIGN_PTR(addr) ((char*)DWORD_PAD((uintptr_t)(addr)))
#define IS_DWORD_ALIGNED(addr) (!((uintptr_t)(addr) & (DWORD_BYTES - 1)))

/*
 * ChunkTag = Int Int Int
 *
 * The K70 machine will only have 128MiB RAM, so an address only needs 27 bits.
 * Double word alignment sets the three low order bits to be 0 always, so
 * size can live in 24 bits (just so long as you shift left 3 to use the value)
 */
typedef struct ChunkTag {
  uint32_t size:24;
  uint32_t pid:7;
  uint32_t free:1;
} ChunkTag;

/*
 * ChunkHeader = ChunkTag (ChunkHeader, ChunkHeader)
 *
 * THe ChunkHeader is a handle to a chunk.
 * The ChunkTag is always a part of a chunk and takes up 4 bytes.
 * When free a chunk will hold two pointers defined by the header.
 */
typedef struct ChunkHeader {
  ChunkTag             tag;
  struct ChunkHeader** prev; /* implement an asymettric list, prev point to a pointer not a node */
  struct ChunkHeader*  next;
} ChunkHeader;

#define BAD_PTR ((ChunkHeader*)0x0)
#define BAD_PPTR ((ChunkHeader**)0x0)

/*
 * ChunkBin = (ChunkHeader, ChunkHeader)
 *
 * Each bin maintains a clean and a dirty list.
 * Chunks on the clean list have been coalesced, and
 * are a multiple of the min chunk size. Plus no
 * two clean chunks are bordering each other in memory.
 * Dirty chunks have no requirements other than they meet
 * the bin size restriction
 */
typedef struct ChunkBin {
  ChunkHeader* clean;
  ChunkHeader* dirty;
} ChunkBin;

/*
 * This is the minimal chunk the allocator will give you.
 * It allows the allocator to keep alignment garuntees, and
 * to leave space for the ChunkHeader when a chunk is free'd.
 *
 * Programs needing to make lots of small allocations are better
 * allocating a large chunk from this allocator then managing the memory 
 * inside of that chunk themselves.
 */
#define MIN_ALLOC_BYTES (sizeof(ChunkHeader) + sizeof(ChunkTag))

/*
 * Macros to access parts of the chunks.
 * A chunk Pred is the chunk immediately preceeding the current chunk in memory.
 * This is not the same as the chunk found by following the ->prev pointer,
 * that chunk is the previous chunk in the chunk list for the bin which the chunk
 * resides.
 * A chuck Succ is the immediate successor of a chunk in memory.
 * Coalescing happens between chunks and their succ/pred chunks.
 */
#define getTag(chk) ((chk)->tag)
#define zeroTag(chk) do{\
        getTag(chk).pid=0;  \
	getTag(chk).size=0; \
	getTag(chk).free=0; \
	}while(0)
#define readSize(tag) (((uint32_t)(tag).size << 3))
#define readSizePtr(tag) (readSize(*(tag)))
#define writeSize(tag,sz) ((tag).size = (((uint32_t)(sz)) >> 3))
#define writeSizePtr(tag,sz) (writeSize(*(tag), (sz)))
#define getSize(chk) (readSize(getTag((chk))))
#define getPayload(chk) ((void*)((char*)(chk) + sizeof(ChunkTag)))
#define getFooter(chk) ((ChunkTag*)((char*)(chk) + getSize((chk)) - sizeof(ChunkTag)))
#define zeroFooter(chk) do{\
	getFooter(chk)->size=0; \
	getFooter(chk)->free=0;\
	}while(0)
#define getSucc(chk) ((ChunkHeader*)((char*)(chk) + getSize((chk))))
#define getPred(chk) ((ChunkHeader*)((char*)(chk) - readSizePtr(getTagPred((chk)))))
#define getTagSucc(chk) ((ChunkTag*)((char*)(chk) + getSize((chk))))
#define getTagPred(chk) ((ChunkTag*)((char*)(chk) - sizeof(ChunkTag)))
#define isUnlinked(chk) ((chk) != BAD_PTR && (chk)->prev == BAD_PPTR && (chk)->next == BAD_PTR)
#define hasCleanChunks(bin) ((bin).clean != BAD_PTR)
#define hasDirtyChunks(bin) ((bin).dirty != BAD_PTR)
#define isExactMatch(chk, sz) ((chk != BAD_PTR) && (((getSize((chk))) == (sz)) || (((getSize((chk))) - (sz)) <= MIN_ALLOC_BYTES)))

/*
 * Set up some needed values about the address space.
 * For the NICE platform these come from a local header.
 * For the K70, this will need to be configured by the assembler
 * most likely
 */
#if defined PLATFORM_NICE
#include "ram.h"
static char *ram0 = NULL;           /* DWORD aligned start of system ram */
static char *ramHighAddress = NULL; /* top of ram offset from ram0       */
#elif defined PLATFORM_K70CW
#include "ramk70.h"
static char *ram0 = NULL;
static char *ramHighAddress = NULL;
#else
#error "No Platform Support"
#endif

#ifndef ALLOCATION_BITMAP_SIZE
#error Cannot compile without ALLOCATION_BITMAP_SIZE being defined
#endif

/*
 * SOme more macros setting up various sizes and boundaries in the allocator
 */
#define MAX_PRE_ALLOCATIONS 16
#define MAX_BINS 128
#define MAX_RANGE_BINS (MAX_BINS - 1)
#define MAX_FAST_BIN 512
#define MAX_RANGE_BIN (1<<26)         /* K70 will have at most 1<<27 addresses */

/*
 * AllocHeader
 *
 * This structure is located at ram0, and is the metadata of the allocator.
 *
 * The free lists are binned on chunk size
 * With fast bins for chunks from size [16,512) spaced 8 bytes apart
 * However all bins can be used if an 'exact' chunk isn't located
 * and so chunks are normalized onto bins 1:127
 * Splits only happen on clean bin chunks. No two clean chunks are adjacent in physical mem.
 */
typedef struct AllocHeader {
  uint32_t lastAllocSize; /* track previous allocation to determine if we should preallocate */
  ChunkBin bins[MAX_BINS];
  char bitmap[ALLOCATION_BITMAP_SIZE];
} AllocHeader;

/*
 * Two values which are needed at runtime, and must be computed then.
 * 'heap' is the base of the heap.
 * 'numChunkOffsets' is the number of MIN_ALLOC_BYTES chunks the memory can 
 * provide. This is used by the bitmap to track allocations.
 */
char *heap;
size_t numChunkOffsets;

uint32_t totalRAM = 0; /* ramHighAddress - heap */
static uint32_t allocHWM = 0; /* high water mark */
static uint32_t allocCount = 0; /* # allocations */
static uint32_t freeCount = 0; /* # frees */
static uint32_t allocInUse = 0; /* bytes allocated */
static uint32_t allocFree = 0; /* bytes released */
static int32_t allocPM = 0; /* +/- count */

/*
 * Macros to address into the bitmap.
 * Basically an address gets scaled onto the (0,numChunkOffsets) range
 * From that we can select the byte of the bitmap buffer to load
 * the bit to operate on.
 */
/* #define getAddrBitmapOffset(addr) (size_t)(((numChunkOffsets * (uintptr_t)((char*)(addr) - heap)) / (uintptr_t)(totalRAM))) */
#define getAddrBitmapOffset(addr) (size_t)(((uintptr_t)addr - (uintptr_t)heap) / MIN_ALLOC_BYTES)
#define getAddrBit(addr) ((DWORD_BYTES - 1) - (getAddrBitmapOffset((addr)) & (DWORD_BYTES - 1)))
#define getAddrByte(addr) (getAddrBitmapOffset((addr)) / DWORD_BYTES)

#define setBitmap(addr) (header->bitmap[getAddrByte((addr))] |= (1 << getAddrBit((addr))))
#define clearBitmap(addr) (header->bitmap[getAddrByte((addr))] &= ~(1 << getAddrBit((addr))))
#define checkBitmap(addr) (header->bitmap[getAddrByte((addr))] & (1 << getAddrBit((addr))))

/*
 * the header is a global value in the allocator
 */
static AllocHeader* header = NULL;

/*
 * Macros for addressing into the bins.
 * Fast bins are sized to exact 8 byte splits up to 512.
 * Range bins can actually address fast bins, and subdivide then
 * entire chunk space.
 */
#define RECENT_CHUNK_BIN (header->bins[0].dirty)
#define REMAINDER_CHUNK_BIN (header->bins[0].clean)
#define isFastBinSize(sz) (((sz) < MAX_FAST_BIN) && !((sz) & (DWORD_BYTES -1)))
#define isRangeBinSize(sz) ((sz) >= MAX_FAST_BIN && (sz) < MAX_RANGE_BIN)
#define isTopBinSize(sz) ((sz) >= MAX_RANGE_BIN)
#define fastBinIndex(sz) (((sz) - MIN_ALLOC_BYTES) / DWORD_BYTES)
#define rangeBinIndex(sz) ((((MAX_RANGE_BINS - 1)*((sz) - MIN_ALLOC_BYTES)) / (MAX_RANGE_BIN - MIN_ALLOC_BYTES)))
#define getBinIndex(sz) (1 + (isFastBinSize((sz)) ? fastBinIndex((sz)) : rangeBinIndex((sz))))
#define getBinByIndex(idx) (header->bins[(idx)])
#define getBin(sz) (getBinByIndex(getBinIndex((sz))))

/*
 * initChunk :: Ptr -> Integer -> ChunkHeader
 *
 * Initializes a pointer of size into a ChunkHeader
 */
ChunkHeader* initChunk(void* mem, size_t size) {
  ASSERT(IS_WORD_ALIGNED(mem) && "Attempt to initialize chunk which does not align to WORD bound");
  ASSERT(IS_WORD_ALIGNED(size) && "Attempt to initialize chunk whose size will misalign successive chunks");
  ASSERT((size >= MIN_ALLOC_BYTES) && "Attempt to initialize chunk of diminutive size");
  ASSERT(!((char*)mem + size > ramHighAddress) && "Attempt to initialize chunk larger than RAM");
  ChunkHeader *chunk = (ChunkHeader*)mem;
  zeroTag(chunk);
  writeSize(getTag(chunk), size);
  writeSizePtr(getFooter(chunk), size);
  getTag(chunk).free = 1;
  getFooter(chunk)->free = 1;
  chunk->prev = BAD_PPTR;
  chunk->next = BAD_PTR;
  return chunk;
}

/*
 * insertChunkBefore :: ChunkHeader -> ChunkHeader -> ()
 *
 * Inserts 'this' before 'that'
 */
void insertChunkBefore(ChunkHeader* that, ChunkHeader* this) {
  ASSERT(that != BAD_PTR && "insertChunkBefore() that is BAD_PTR");
  ASSERT(this != BAD_PTR && "insertChunkBefore() this is BAD_PTR");
  ASSERT((that->prev != BAD_PPTR) && "Cannot insert when 'that' node isn't in a list");
  this->prev = that->prev;
  *(this->prev) = this;
  this->next = that;
  that->prev = &this->next;
}

/*
 * insertAfter :: ChunkHeader -> ChunkHeader -> ()
 *
 * Inserts 'this' after 'that'
 */
static void insertChunkAfter(ChunkHeader* that, ChunkHeader* this) {
  ASSERT(that != BAD_PTR && "insertChunkAfter() that is BAD_PTR");
  ASSERT(this != BAD_PTR && "insertChunkAfter() this is BAD_PTR");
  this->next = that->next;
  if (this->next != BAD_PTR)
    this->next->prev = &this->next;
  that->next = this;
  this->prev = &that->next;
}

/*
 * removeChunk :: ChunkHeader -> ChunkHeader
 *
 * Removes 'this' from its list and returns it.
 */
ChunkHeader* removeChunk(ChunkHeader* this) {
  ASSERT(this != NULL && "removeChunk() this is NULL");
  *(this->prev) = this->next;
  if (this->next != BAD_PTR)
    this->next->prev = this->prev;
  return this;
}

/*
 * Mode flags to operatre on the different bin types
 */
typedef enum {
  BinDirty,
  BinClean,
  BinRecent,
  BinLastSplitRem
} BinChunkMode;

/*
 * binChunk :: ChunkHeader -> BinChunkMode -> ()
 *
 * Adds a chunk to a bin. Bins are sorted in address order.
 * Except the recent bin which is a stack and the lastSplitRem
 * which is a unit.
 */
static void binChunk(ChunkHeader* chunk, BinChunkMode mode) {
  ChunkBin* bin;
  ChunkHeader* chunks = BAD_PTR;

  ASSERT(isUnlinked(chunk) && "Chunk has not been unlinked prior to bining");

  bin = &getBin(getSize(chunk));
  switch(mode) {
  case BinDirty:
    if ((chunks = bin->dirty) == BAD_PTR) {
      bin->dirty = chunk;
      chunk->prev = &bin->dirty;
      return;
    }
    break;
  case BinClean:
    if ((chunks = bin->clean) == BAD_PTR) {
      bin->clean = chunk;
      chunk->prev = &bin->clean;
      return;
    }
    break;
  case BinRecent:
    if ((chunks = RECENT_CHUNK_BIN) == BAD_PTR) {
      RECENT_CHUNK_BIN = chunk;
      chunk->prev = &RECENT_CHUNK_BIN;
    } else {
      insertChunkBefore(chunks, chunk);
    }
    return;
  case BinLastSplitRem:
    /*
    if ((chunks = REMAINDER_CHUNK_BIN) == BAD_PTR) {
      REMAINDER_CHUNK_BIN = chunk;
      chunk->prev = &REMAINDER_CHUNK_BIN;
    } else {
      insertChunkBefore(chunks, chunk);
    }
    */
    ASSERT(REMAINDER_CHUNK_BIN == BAD_PTR && "binChunk() unexpected value in REMAINDER_CHUNK_BIN");
    REMAINDER_CHUNK_BIN = chunk;
    chunk->prev = &REMAINDER_CHUNK_BIN;
    return;
  }

  while (chunks != BAD_PTR) {
    if ((intptr_t)chunk < (intptr_t)chunks) {
      insertChunkBefore(chunks, chunk);
      break;
    } else if (chunks->next == BAD_PTR) {
      insertChunkAfter(chunks, chunk);
      break;
    } else {
      chunks = chunks->next;
    }
  }

  return;
}

static volatile const ChunkHeader* assertChunkAddr     = NULL;
static volatile const ChunkHeader* assertChunkPredAddr = NULL;
static volatile const ChunkHeader* assertChunkSuccAddr = NULL;

static void assertChunk(ChunkHeader* chunk) {
    int predFree    = getTagPred(chunk)->free;
    size_t predSize = readSizePtr(getTagPred(chunk));
    int succFree    = getTagSucc(chunk)->free;
    size_t succSize = readSizePtr(getTagSucc(chunk));

    ChunkHeader* pred = getPred(chunk);
    ChunkHeader* succ = getSucc(chunk);

    assertChunkAddr = chunk;
    assertChunkPredAddr = pred;
    assertChunkSuccAddr = succ;

    int firstChunk = ((void*)heap == (void*)chunk);
    int lastChunk  = (((char*)chunk + getSize(chunk)) == ramHighAddress);

    if (firstChunk)
        sysprintln("validateChunk() first chunk 0x%08" PRIx32 "", (intptr_t)chunk);

    if (lastChunk)
        sysprintln("validateChunk() last chunk 0x%08" PRIx32 "", (intptr_t)chunk);

    if (! firstChunk) {
        int predSizeConsistent = (getSize(pred) == readSizePtr(getFooter(pred)));
        ASSERT(predSizeConsistent && "validateChunk() pred size consistency error");

        int predSizeOK = (getSize(pred) == predSize);
        ASSERT(predSizeOK && "validateChunk() pred size mismatch");

        int predFreeConsistent = (getTag(pred).free == getFooter(pred)->free);
        ASSERT(predFreeConsistent && "validateChunk() pred free consistency error");

        int predFreeOK = (getTag(pred).free == predFree);
        ASSERT(predFreeOK && "validateChunk() pred free mismatch");

        int predInBitmap = checkBitmap(getPayload(pred));
        ASSERT((predFree || predInBitmap) && "validateChunk() pred bitmap error");
    }

    if (! lastChunk) {
        int succSizeConsistent = (getSize(succ) == readSizePtr(getFooter(succ)));
        ASSERT(succSizeConsistent && "validateChunk() succ size consistency error");

        int succSizeOK = (getSize(succ) == succSize);
        ASSERT(succSizeOK && "validateChunk() succ size mismatch");

        int succFreeConsistent = (getTag(succ).free == getFooter(succ)->free);
        ASSERT(succFreeConsistent && "validateChunk() succ free consistency error");

        int succFreeOK = (getTag(succ).free == succFree);
        ASSERT(succFreeOK && "validateChunk() succ free mismatch");

        int succInBitmap = checkBitmap(getPayload(succ));
        ASSERT((succFree || succInBitmap) && "validateChunk() succ bitmap error");
    }
}

/*
 * initRam :: ()
 *
 * On platforms where the starting address of allocatable memory
 * is not able to be aligned on a double word boundary the base
 * of the ram must be adjusted at runtime.
 */
static void initRam(void) {
  if (ram0 == NULL) {
    ram0 = (char*)SDRAM_START;
    ramHighAddress = (char*)SDRAM_END;

    /* Zero out the header RAM */
    kmemset(ram0, 0, sizeof *header);
    
    /* Overlay the header at the base of ram */
    header = (struct AllocHeader*)ram0;

    /* invalidate bins */
    for (unsigned i = 0; i < MAX_BINS; i++) {
        header->bins[i].dirty = BAD_PTR;
        header->bins[i].clean = BAD_PTR;
    }

    /* align things for the heap. Since a chunk has a WORD sized tag at boths ends
     * and our allocator is required to return pointers which align on DOUBLE WORD
     * boundaries we need to push the start of the heap to a WORD aligned address
     * so that the first chunk user data will be on a DOUBLE WORD address.
     */
    heap = ram0 + WORD_PAD(sizeof(struct AllocHeader));
    
    totalRAM = (uint32_t)(ramHighAddress - heap);
    
    /* invalidate the heap */
    kmemset(heap, 0xfa, totalRAM);
    numChunkOffsets = totalRAM / MIN_ALLOC_BYTES;

    ChunkHeader* firstChunk = initChunk(heap, totalRAM);
    getBinByIndex(MAX_BINS - 1).clean = firstChunk;
    firstChunk->prev = &getBinByIndex(MAX_BINS - 1).clean;
    
    allocFree = getSize(firstChunk);
  }
  return;
}

/*
 * unlinkChunk :: [ChunkHeader] -> [ChunkHeader]
 *
 * Removes the chunk from its linked list.
 * Patches up the prev and next chunks.
 */
static ChunkHeader* unlinkChunk(ChunkHeader* chunk) {
  if (chunk->prev != BAD_PPTR || chunk->next != BAD_PTR) {
    chunk = removeChunk(chunk);
    chunk->prev = BAD_PPTR;
    chunk->next = BAD_PTR;
  }
  return chunk;
}

/*
 * coalesce :: [ChunkHeader] -> ()
 *
 * Performs a coalescing pass over the provided list.
 * Chunks are scanned for free pred and succ chunks, and merged
 * appropriately. The new larger chunk is placed in a new bin
 */
static void __attribute__((used)) coalesce(ChunkHeader* chunks) {
  ChunkHeader* chunk = chunks;

  while (chunk != BAD_PTR) {
    ChunkHeader* next = chunk->next;
    ASSERT(!checkBitmap(getPayload(chunk)) && "kmalloc() coalescing chunk exists in bitmap");

    /* Case 1: both pred and succ are free */
    if (getTagPred(chunk)->free && getTagSucc(chunk)->free) {
      ChunkHeader* pred = getPred(chunk);
      ChunkHeader* succ = getSucc(chunk);
      ASSERT(!checkBitmap(getPayload(pred)) && "kmalloc() coalescing(1) pred exits in bitmap");
      ASSERT(!checkBitmap(getPayload(succ)) && "kmalloc() coalescing(1) succ exits in bitmap");
      ASSERT(getTag(pred).free && "Tag ismatch in pred coalesce(1)");
      ASSERT(getTag(succ).free && "Tag mismatch in succ coalesce(1)");
      writeSize(getTag(pred), getSize(pred) + getSize(chunk) + getSize(succ));
      writeSizePtr(getFooter(chunk), getSize(pred));
      unlinkChunk(chunk);
      unlinkChunk(succ);
      zeroFooter(succ);
      zeroTag(chunk);
      zeroFooter(chunk);
      zeroTag(succ);
      binChunk(unlinkChunk(pred), BinClean);
    } else if (getTagPred(chunk)->free) { /* Case 2: only pred is free */
      ChunkHeader* pred = getPred(chunk);
      ASSERT(!checkBitmap(getPayload(pred)) && "kmalloc() coalescing(1) pred exits in bitmap");
      ASSERT(getTag(pred).free && "Tag mismatch in pred coalesce(2)");
      writeSize(getTag(pred), getSize(pred) + getSize(chunk));
      writeSizePtr(getFooter(chunk), getSize(pred));
      unlinkChunk(chunk);
      zeroFooter(pred);
      zeroTag(chunk);
      binChunk(unlinkChunk(pred), BinClean);
    } else if (getTagSucc(chunk)->free) { /* Case 3: only succ is feee */
      ChunkHeader* succ = getSucc(chunk);
      ASSERT(!checkBitmap(getPayload(succ)) && "kmalloc() coalescing(1) succ exits in bitmap");
      ASSERT(getTag(succ).free && "Tag mismatch in succ coalesce(3)");
      writeSize(getTag(chunk), getSize(chunk) + getSize(succ));
      writeSizePtr(getFooter(succ), getSize(chunk));
      unlinkChunk(succ);
      zeroFooter(chunk);
      zeroTag(succ);
      binChunk(unlinkChunk(chunk), BinClean);
    } else { /* Case 4: isolated chunk */
      /* Ignore uncoalescable case for now */
    }

    chunk = next;
  }
}

typedef enum {
  ExactFitDontRebin,
  ExactFitDoRebin
} ExactFitRebinFlag;

/* exactFitSearch :: [ChunkHeader] -> Integet -> Bool -> Maybe ChunkHeader
 *
 * Does an 'exact' fit search through the chunk list and returns
 * a match, or NULL when search fails.
 */
static ChunkHeader* exactFitSearch(ChunkHeader* chunks, size_t size, ExactFitRebinFlag rebinMismatch) {
  ChunkHeader* chunk = chunks;

  while (chunk != BAD_PTR) {
    if (isExactMatch(chunk, size)) {
      return chunk;
    }
    if (rebinMismatch) {
      ChunkHeader* next = chunk->next;
      binChunk(unlinkChunk(chunk), BinDirty);
      chunk = next;
    } else {
      chunk = chunk->next;
    }
  }
  return BAD_PTR;
}

/*
 * firstFitSearch :: [ChunkHeader] -> Integer -> Maybe ChunkHeader
 *
 * Does a first fit search though the chunk list and returns
 * a match, or NULL when search fails.
 */
static ChunkHeader* firstFitSearch(ChunkHeader* chunks, size_t size) {
  ChunkHeader* chunk = chunks;

  while (chunk != BAD_PTR) {
    if (getSize(chunk) >= size) {
      return chunk;
    }
    chunk = chunk->next;
  }
  return BAD_PTR;
}

/*
 * splitChunk :: ChunkHeader -> Integer -> ChunkHeaderPtr -> ChunkHeader
 *
 * Cleaves a chunk of size 'size' from 'chunk' and another chunk 'rest'
 * which is the remainder. 'rest' is returned via an out pointer.
 * the new chunk is returned.
 */
ChunkHeader* splitChunk(ChunkHeader* chunk, size_t size, ChunkHeader** rest) {
  ASSERT((getSize(chunk) > size) && "Something is wrong. Splitting from too small a chunk");
  ASSERT(IS_WORD_ALIGNED(size) && "Something is wrong. Size is not WORD aligned");
  ASSERT(IS_WORD_ALIGNED(getSize(chunk)) && "Something is wrong. Chunk is not WORD aligned");

  size_t oldSize = getSize(chunk);
  ChunkHeader* chunkA = initChunk(chunk, size);
  ChunkHeader* chunkB = initChunk((char*)chunk + size, oldSize - size);

  ASSERT(IS_WORD_ALIGNED(chunkA) && "Split chunk A is not on a WORD boundary");
  ASSERT(IS_WORD_ALIGNED(chunkB) && "Split chunk B is not on a WORD boundary");
  ASSERT((getSize(chunkA) == size) && "Something is wrong. Split chunk A size incorrect");
  ASSERT((readSizePtr(getFooter(chunkA)) == size) && "Something is wrong. Split chunk A footer size incorrect");
  ASSERT((getSize(chunkB) == (oldSize - size)) &&"Something is wrong. Split chunk B size incorrect");
  ASSERT((readSizePtr(getFooter(chunkB)) == (oldSize - size)) && "Something is wrong. Split chunk B size incorrect");
  *rest = chunkB;
  return chunkA;
}

/*
 * allocateChunk :: Integer -> ChunkHeader
 *
 * The main allocation function, 'size' must be properly padded to
 * allow this chunk to hold a ChunkHeader and ChunkTag footer
 * once it has been free'd
 */
static ChunkHeader* allocateChunk(size_t size) {
  ChunkHeader* chunk = BAD_PTR;
  ChunkHeader* rest = BAD_PTR;

  /* Step 1: First check if the last free'd block is suitable
   *         does not exceed the requested size by no more than
   *         MIN_ALLOC_BYTES
   */
  if (isExactMatch(RECENT_CHUNK_BIN, size)) {
    chunk = unlinkChunk(RECENT_CHUNK_BIN);
    goto exit;
  }

  /* Step 2: Compute this chunks bin and locate an exact match if possible
   *
   * Use this time to coalesce too.
   */
  if ((chunk = exactFitSearch(getBin(size).dirty, size, ExactFitDontRebin)) != BAD_PTR) {
    chunk = unlinkChunk(chunk);
    goto exit;
  }

  /* Only coalesce if there are dirty chunks, but no match */
  /*
  if (getBin(size).dirty != BAD_PTR) {
    coalesce(getBin(size).dirty);
    struct ChunkHeader *rb = RECENT_CHUNK_BIN;
    coalesce(rb);
  }
  */

  /* Step 3: See if there is an exact chunk anywhere in the recent bin.
   *         Failed matches get pushed onto a dirty bin list of the correct size
   */
  if ((chunk = exactFitSearch(RECENT_CHUNK_BIN, size, ExactFitDoRebin)) != BAD_PTR) {
    chunk = unlinkChunk(chunk);
    goto exit;
  }

  /* Step 4: Check if the last split produced a remainder which can be used
   *         to satisfy this allocation
   */
  if ((chunk = firstFitSearch(REMAINDER_CHUNK_BIN, size)) != BAD_PTR) {
    chunk = unlinkChunk(chunk);
    goto split;
  }

  /* Step 5: Does the bin have a chunk in its clean list */
  if ((chunk = firstFitSearch(getBin(size).clean, size)) != BAD_PTR) {
    chunk = unlinkChunk(chunk);
    goto split;
  }

  /* Step 6: Scan all larger bins for a chunk ... */
  for (int i = getBinIndex(size) + 1; i < MAX_BINS; i++) {
    if ((chunk = firstFitSearch(getBinByIndex(i).clean, size)) != BAD_PTR) {
      chunk = unlinkChunk(chunk);
      goto split;
    }
  }

  /* Step 7: Coalesce space until a fit is found */
  for (int i = getBinIndex(size) + 1; i < MAX_BINS; i++) {
    /*
    coalesce(getBinByIndex(i).dirty);
    */
    if ((chunk = firstFitSearch(getBinByIndex(i).clean, size)) != BAD_PTR) {
      chunk = unlinkChunk(chunk);
      goto split;
    }
  }

  /* Step 8: Get more memory from system ... oh wait! */
  goto exit;

split:
  /* Don't split if we would leave a chunk which cannot be reallocated */
  if (getSize(chunk) - size < MIN_ALLOC_BYTES) {
    goto exit;
  }
  /* Step 9: carve off a chunk of memory */
  chunk = splitChunk(chunk, size, &rest);


  size_t sizeRest = getSize(rest);
  if (header->lastAllocSize == size) {
    for (int i = 0; i < MAX_PRE_ALLOCATIONS && sizeRest > size && (sizeRest - size) >= MIN_ALLOC_BYTES; i++) {
      ChunkHeader* pre = splitChunk(rest, size, &rest);
      binChunk(pre, BinRecent);
      sizeRest = getSize(rest);
    }
  }

  binChunk(rest, BinLastSplitRem);

exit:

  return chunk;
}

static void* __kmalloc(size_t size, int pid) {
  void* mem = NULL;
  size_t newSize = size + (2 * sizeof(ChunkTag));

  initRam();
  
  if (newSize < MIN_ALLOC_BYTES) {
    newSize += (MIN_ALLOC_BYTES - newSize);
  }

  ChunkHeader* chunk = allocateChunk(DWORD_PAD(newSize));

  if (chunk != BAD_PTR) {
    mem = getPayload(chunk);

    header->lastAllocSize = getSize(chunk);

    /* store the PID of the caller in the previously reserved byte just before the footer */
    getTag(chunk).pid = pid;
    getFooter(chunk)->pid = pid;

    /* indicate this chunk is allocated */
    getTag(chunk).free = 0;
    getFooter(chunk)->free = 0;

    /* tag this address as allocated in the bitmap */
    ASSERT(!checkBitmap(mem) && "Memory error. Cannot allocate adress already allocated.");
    
    /* DEBUG: Validate each 16k stride */
    for (char* iter = mem; iter < ((char*)mem + size); iter += MIN_ALLOC_BYTES) {
        ASSERT(!checkBitmap(iter) && "CORRUPT REGION DETECTED! DOUBLE ALLOCATION ERROR");
    }

    setBitmap(mem);
    
    allocInUse += getSize(chunk);
    allocFree -= getSize(chunk);
    allocPM++;
    allocCount++;
    if (allocInUse > allocHWM)
      allocHWM = allocInUse;
    
    kmemset(mem, 0, size);
  } else {
    errno = ENOMEM;
  }
  return mem;
}

/*
 * kmalloc :: Integer -> Ptr
 *
 * allocate chunk of at least 'size' size.
 * Returned pointer will be double word aligned.
 * If size is 0, a chunk of MIN_ALLOC_BYTES is returned.
 */
void* kmalloc(size_t size) {
    lock(&malLock);
    void* mem = __kmalloc(size, getpid());
    unlock(&malLock);
    return mem;
}

void* syskmalloc(size_t size) {
    syslock(&malLock);
    void* mem = __kmalloc(size, getpid());
    sysunlock(&malLock);
    return mem;
}

void* syskmalloc0(size_t size) {
    syslock(&malLock);
    void* mem = __kmalloc(size, 0);
    sysunlock(&malLock);
    return mem;
}

static void __kfree(void* ptr) {
  /*
   * Safety check #0: NULL has no effect.
   */
  if (! ptr) return;

  /*
   * Safety check #1: A valid pointer from malloc will be aligned to DWORD_BYTES
   */
  int isAligned = !((uintptr_t)ptr & (DWORD_BYTES - 1)); /* check the low bits are zero */
  if (isAligned) {
    /*
     * Safetey check #2: A valid pointer from malloc will have its address recorded in
     * the bitmap. This is the address that is given to the malloc caller.
     */
    ASSERT(checkBitmap(ptr) && "Memory error. Cannot free address not allocated by malloc");
   
    /*
     * Safetey check #3: We only allocate addresses between heap and ramHighAddress
     */
    ASSERT(((char*)ptr >= heap) && ((char*)ptr <= ramHighAddress) && "Memory error. Address out of allocator zone");
    if (checkBitmap(ptr)) {
      ChunkHeader* chunk = (ChunkHeader*)((uintptr_t)ptr - sizeof(ChunkTag));
      assertChunk(chunk);

      size_t chunkSize = getSize(chunk);
      allocInUse -= chunkSize;
      allocFree += chunkSize;
      freeCount++;
      allocPM--;
      getTag(chunk).pid = 0;
      getTag(chunk).free = 1;
      getFooter(chunk)->pid = 0;
      getFooter(chunk)->free = 1;
      chunk->prev = BAD_PPTR;
      chunk->next = BAD_PTR;
      clearBitmap(ptr);
      binChunk(chunk, BinRecent);
    }
  }
}

/*
 * kfree :: Ptr -> ()
 *
 * free allocated chunk. If ptr is NULL or not from malloc, this is a noop.
 */
void kfree(void* ptr) {
    lock(&malLock);
    __kfree(ptr);
    unlock(&malLock);
}

/**
 * syskfree :: Pre -> ()
 *
 * like kfree, but callable from handler
 */
void syskfree(void* ptr) {
    syslock(&malLock);
    __kfree(ptr);
    sysunlock(&malLock);
}



/*
 * hexdump :: Ptr -> Integer -> FILE* -> ()
 *
 * Hexdump a buffer to 'out'
 */
static void hexdump(void* buf, size_t length) {
  char ascii[33] = {0};
  char *p = (char*)buf;
  unsigned i;
  for (i = 0; i < length; i++) {
    if (!(i % 32)) {
      if (i != 0) {
        fprintln(rp->tty, "  %s", ascii);
      }

      fprint(rp->tty, " %.8" PRIxPTR " ", (uintptr_t)heap + i);
    }

    fprint(rp->tty, " %.2x", (uint8_t)p[i]);

    if ((p[i] >= 0x20) && (p[i] <= 0x7e)) {
      ascii[i % 32] = p[i];
    } else {
      ascii[i % 32] = '.';
    }
    ascii[(i % 32) + 1] = '\0';
  }

  while ((i++ % 32) != 0) {
    fputstr(rp->tty, "  ");
  }

  fprintln(rp->tty, " %s", ascii);
}

/*
 * dumpChunk :: FILE* -> ChunkHeader -> Bool -> ()
 *
 * Dumps a chunk to out
 */
void dumpChunk(ChunkHeader* chunk, int doHexdump) {
  fputstr(rp->tty, "**** YAMalloc Chunk Dump ****\n\n");
  fputstr(rp->tty, "General Info:\n\n");
  fprintln(rp->tty, "    Chunk addr: 0x%08" PRIxPTR "\n", (uintptr_t)chunk);
  
  uintptr_t mem = (uintptr_t)getPayload(chunk);
  fprintln(rp->tty, "    Chunk user addr: 0x%08" PRIxPTR "", mem);
  fprintln(rp->tty, "    Chunk bitmap addreds (off: %d,byte: %d,bit: %d)", getAddrBitmapOffset(mem), getAddrByte(mem), getAddrBit(mem));
  fprintln(rp->tty, "    Chunk user addr DWORD aligned?: %s", IS_DWORD_ALIGNED(getPayload(chunk)) ? "yes" : "no");
  
  uint32_t st = readSize(getTag(chunk)), sf = readSizePtr(getFooter(chunk));
  fprintln(rp->tty, "    Chunk size: %" PRIu32 " (%" PRIu32 ")%s", st, sf, st == sf ? "" : " MISMATCH!");
  
  int ft = getTag(chunk).free, ff = getFooter(chunk)->free;
  fprintln(rp->tty, "    Chunk isFree: %d (%d)%s", ft, ff, ft == ff ? "" : " MISMATCH!");
  
  fprintln(rp->tty, "    Sanity Checks. Pred Size: %" PRIu32 ", Free: %d", readSizePtr(getTagPred(chunk)), getTagPred(chunk)->free);
  fprintln(rp->tty, "                   Succ Size: %" PRIu32 ", Free: %d", readSizePtr(getTagSucc(chunk)), getTagSucc(chunk)->free);

  int isFree = getTag(chunk).free;

  if (!isFree) {
	if (!checkBitmap(mem)) {
	  fputstr(rp->tty, "    WARNING: Allocated chunk NOT recorded in BITMAP!\n");
	}
    fprintln(rp->tty, "    Chunk PID: %d (%d)", getTag(chunk).pid, getFooter(chunk)->pid);
  } else {
	if (checkBitmap(mem)){
	  fputstr(rp->tty, "    WARNING: Free chunk HAS record in BITMAP!\n");
	}
    fprintln(rp->tty, "    Chunk Prev Ptr: 0x%08" PRIxPTR "", (uintptr_t)chunk->prev);
    fprintln(rp->tty, "    Chunk Next Ptr: 0x%08" PRIxPTR "", (uintptr_t)chunk->next);
  }
  
  if (doHexdump) {
    fputstr(rp->tty, "Memory Dump:\n\n");
    hexdump((void*)chunk, getSize(chunk));
  }
}

/*
 * kmallocDump :: FILE* -> ()
 *
 * Dumps the current state of the allocator and its data structures to 'out'
 */
void kmallocDump(void) {
  initRam(); /* incase we haven't initialized the memory already */

  fputstr(rp->tty, "**** YAMalloc Memory Dump ****\n\n");
  fputstr(rp->tty, "General Info:\n\n");
  fprintln(rp->tty, "    Addr ram0:    0x%08" PRIxPTR "", (uintptr_t)ram0);
  fprintln(rp->tty, "    Addr ramHigh: 0x%08" PRIxPTR "", (uintptr_t)ramHighAddress);
  fprintln(rp->tty, "    Min. Allocation size (B): %d", MIN_ALLOC_BYTES);
  fputstr(rp->tty, "\n");
  fputstr(rp->tty, "Allocator Header Info:\n\n");
  fprintln(rp->tty, "    # Chunk Offsets In Bitmap: %d", numChunkOffsets);
  fprintln(rp->tty, "    Size of bin area (B) : %d", sizeof(header->bins[0]) * MAX_BINS);
  fprintln(rp->tty, "    Size of Bitmap (B)   : %d", ALLOCATION_BITMAP_SIZE);
  fprintln(rp->tty, "    Size of Header (B)   : %d", sizeof(struct AllocHeader));
  fprintln(rp->tty, "    Addr of header (should be ram0): 0x%08" PRIxPTR "", (uintptr_t)header);
  fprintln(rp->tty, "    Addr of bin 0                  : 0x%08" PRIxPTR "", (uintptr_t)header->bins);
  fprintln(rp->tty, "    Addr of bin 127                : 0x%08" PRIxPTR "", (uintptr_t)header->bins + MAX_BINS);
  fprintln(rp->tty, "    Addr of bitmap start           : 0x%08" PRIxPTR "", (uintptr_t)header->bitmap);
  fprintln(rp->tty, "    Addr of bitmap end             : 0x%08" PRIxPTR "", (uintptr_t)header->bitmap + ALLOCATION_BITMAP_SIZE);
  fprintln(rp->tty, "    Addr of header end             : 0x%08" PRIxPTR "", (uintptr_t)((char*)header + sizeof(struct AllocHeader)));
  fprintln(rp->tty, "    Addr of heap start : 0x%08" PRIxPTR "", (uintptr_t)heap);
  fprintln(rp->tty, "    Size of heap (B)   : %" PRIuPTR "", (uintptr_t)(totalRAM));
  fprintln(rp->tty, "    Last address is DWORD aligned : %s", (IS_DWORD_ALIGNED(ramHighAddress) ? "yes" : "no"));
  fputstr(rp->tty, "\n");
  fputstr(rp->tty, "Bitmap Info:\n\n");

#if 0
  for (int i = 0; i < ALLOCATION_BITMAP_SIZE; i++) {
    if (!(i % 10)) {
      if (i != 0) {
        fputstr(rp->tty, "\n");
      }

      fprint(rp->tty, " %.8" PRIxPTR " ", (uintptr_t)header->bitmap + i);
    } else if (i != 0) {
      fputstr(rp->tty, " ");
    }

    char c = header->bitmap[i];
    for (int j = 8; j > 0; j--) {
      fputchar(rp->tty, '.' + (3 * ((c >> (j - 1)) & 1))); /* unset print '.', set print '1' (hence the multiple of 3) */
    }
  }
  fputstr(rp->tty, "\n");
#endif

  fputstr(rp->tty, "Heap Info:\n\n");

  for (uintptr_t i = (uintptr_t)heap; i < (uintptr_t)ramHighAddress; ) {
    fprintln(rp->tty, "** Chunk Offset 0x%08" PRIxPTR "", i);
    struct ChunkHeader *chunk = (struct ChunkHeader*)i;

    if (getSize(chunk) == 0)
      break;

    if ((i + getSize(chunk)) > (uintptr_t)ramHighAddress) {
      fprintln(rp->tty, "** Chunk has potentially corrupt size of %" PRIu32 "", getSize(chunk));
      break;
    }

    dumpChunk(chunk, 0);
    i += getSize(chunk);
  }

  fputstr(rp->tty, "\n");
  fputstr(rp->tty, "Bin Info:\n\n");

  for (int i = 0; i < MAX_BINS; i++) {
    if (i == 0) {
      struct ChunkHeader *chunks = RECENT_CHUNK_BIN;
      for (int j = 0; chunks != BAD_PTR; j++) {
        if (j == 0) {
          fputstr(rp->tty, "** SPECIAL BIN: Recent Chunks\n\n");
        }
        fprint(rp->tty, "**** Recent Chunks [%d] ****\n\n", j);
        dumpChunk(chunks, 0);
        chunks = chunks->next;
      }

      chunks = REMAINDER_CHUNK_BIN;
      for (int j = 0; chunks != BAD_PTR; j++) {
        if (j == 0) {
          fputstr(rp->tty, "** SPECIAL BIN: Last Split Remainders\n\n");
        }
        fprint(rp->tty, "**** Last Split Remainder Chunk [%d] ****\n\n", j);
        dumpChunk(chunks, 0);
        chunks = chunks->next; 
      }
    } else {
      struct ChunkHeader *chunks = getBinByIndex(i).dirty;
      for (int j = 0; chunks != BAD_PTR; j++) {
        if (j == 0) {
          fprint(rp->tty, "** DIRTY BIN %d\n\n", i);
        }
        fprint(rp->tty, "**** Bin %d Chunk [%d] ****\n\n", i, j);
        dumpChunk(chunks, 0);
        chunks = chunks->next;
      }

      chunks = getBinByIndex(i).clean;
      for (int j = 0; chunks != BAD_PTR; j++) {
        if (j == 0) {
          fprint(rp->tty, "** CLEAN BIN %d\n\n", i);
        }
        fprint(rp->tty, "**** Bin %d Chunk [%d] ****\n\n", i, j);
        dumpChunk(chunks, 0);
        chunks = chunks->next;
      }
    }
  }
}

/*
 * This function is a debug routine that is meant to verify the correctness of the
 * bitmap set/check functions used by the allocator.
 *
 * It is a response to a issue seen only on the k70 where two allocations appeared
 * to map to the same bit even though their addresses were > 16 bytes from each 
 * other (something that testing off the device has shown should be an impossibility)
 *
 * The premise here is to make a pass over all possible 16 byte address boundaries in
 * the memory space (from heap to heap + totalRAM) and check and set the appropriate 
 * bit. If the bit has been previously set we output the address in stars.
 */
void kmallocBitmapFunctionIntegrityCheck(void) {
    char bitmap[ALLOCATION_BITMAP_SIZE] = {0}; /* Initialize bitmap to all clear */
    for (uintptr_t addr = (uintptr_t)heap; addr < (uintptr_t)(heap + totalRAM); addr += MIN_ALLOC_BYTES) {
        size_t offset = getAddrBitmapOffset(addr);
        unsigned byte = getAddrByte(addr);
        unsigned bit  = getAddrBit(addr);

        int isSet = bitmap[byte] & (1 << bit);
        if (isSet)
            fprintln(rp->tty, "**** 0x%.8" PRIx32 " **** (%" PRIu32 ", %d, %d)", addr, offset, byte, bit);
        else
            fprintln(rp->tty, "     0x%.8" PRIx32 "      (%" PRIu32 ", %d, %d)", addr, offset, byte, bit);

        bitmap[byte] |= (1 << bit);
    }
}
