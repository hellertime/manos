/**
 * malloc.c - a C memory allocater library targeting the K70 platform
 *
 * Single threaded allocator. Stores free chunks in segregated lists.
 * Uses bitmap to protect memory space.
 * Coalesces only when needed for performance.
 */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <stdio.h>

#include <libc.h>

#include <manos/util.h>

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
struct ChunkTag {
  uint32_t size:24;
  uint32_t pid:7;
  uint32_t free:1;
};

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
struct ChunkBin {
  struct ChunkHeader *clean;
  struct ChunkHeader *dirty;
};

/*
 * ChunkHeader = ChunkTag (ChunkHeader, ChunkHeader)
 *
 * THe ChunkHeader is a handle to a chunk.
 * The ChunkTag is always a part of a chunk and takes up 4 bytes.
 * When free a chunk will hold two pointers defined by the header.
 */
struct ChunkHeader {
  struct ChunkTag tag;
  struct ChunkHeader **prev; /* implement an asymettric list, prev point to a pointer not a node */
  struct ChunkHeader *next;
};

/*
 * This is the minimal chunk the allocator will give you.
 * It allows the allocator to keep alignment garuntees, and
 * to leave space for the ChunkHeader when a chunk is free'd.
 *
 * Programs needing to make lots of small allocations are better
 * allocating a large chunk from this allocator then managing the memory 
 * inside of that chunk themselves.
 */
#define MIN_ALLOC_BYTES (sizeof(struct ChunkHeader) + sizeof(struct ChunkTag))

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
	getTag(chk).size=0; \
	getTag(chk).free=0; \
	}while(0)
#define readSize(tag) (((uint32_t)(tag).size << 3))
#define readSizePtr(tag) (readSize(*(tag)))
#define writeSize(tag,sz) ((tag).size = (((uint32_t)(sz)) >> 3))
#define writeSizePtr(tag,sz) (writeSize(*(tag), (sz)))
#define getSize(chk) (readSize(getTag((chk))))
#define getPayload(chk) ((void*)((char*)(chk) + sizeof(struct ChunkTag)))
#define getFooter(chk) ((struct ChunkTag*)((char*)(chk) + getSize((chk)) - sizeof(struct ChunkTag)))
#define zeroFooter(chk) do{\
	getFooter(chk)->size=0; \
	getFooter(chk)->free=0;\
	}while(0)
#define getSucc(chk) ((struct ChunkHeader*)((char*)(chk) + getSize((chk))))
#define getPred(chk) ((struct ChunkHeader*)((char*)(chk) - readSizePtr(getTagPred((chk)))))
#define getTagSucc(chk) ((struct ChunkTag*)((char*)(chk) + getSize((chk))))
#define getTagPred(chk) ((struct ChunkTag*)((char*)(chk) - sizeof(struct ChunkTag)))
#define isUnlinked(chk) (!((chk)->prev || (chk)->next))
#define hasCleanChunks(bin) ((bin).clean != NULL)
#define hasDirtyChunks(bin) ((bin).dirty != NULL)
#define isExactMatch(chk, sz) ((chk) && (((getSize((chk))) == (sz)) || (((getSize((chk))) - (sz)) <= MIN_ALLOC_BYTES)))

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
struct AllocHeader {
  uint32_t lastAllocSize; /* track previous allocation to determine if we should preallocate */
  struct ChunkBin bins[MAX_BINS];
  char bitmap[ALLOCATION_BITMAP_SIZE];
};

/*
 * Two values which are needed at runtime, and must be computed then.
 * 'heap' is the base of the heap.
 * 'numChunkOffsets' is the number of MIN_ALLOC_BYTES chunks the memory can 
 * provide. This is used by the bitmap to track allocations.
 */
static char *heap;
static size_t numChunkOffsets;

/*
 * Macros to address into the bitmap.
 * Basically an address gets scaled onto the (0,numChunkOffsets) range
 * From that we can select the byte of the bitmap buffer to load
 * the bit to operate on.
 */
#define getAddrBitmapOffset(addr) (size_t)(((numChunkOffsets * (uintptr_t)((char*)(addr) - heap)) / (uintptr_t)(ramHighAddress - heap)))
#define getAddrBit(addr) ((DWORD_BYTES - 1) - (getAddrBitmapOffset((addr)) & (DWORD_BYTES - 1)))
#define getAddrByte(addr) (getAddrBitmapOffset((addr)) / DWORD_BYTES)

#define setBitmap(addr) (header->bitmap[getAddrByte((addr))] |= (1 << getAddrBit((addr))))
#define clearBitmap(addr) (header->bitmap[getAddrByte((addr))] &= ~(1 << getAddrBit((addr))))
#define checkBitmap(addr) (header->bitmap[getAddrByte((addr))] & (1 << getAddrBit((addr))))

/*
 * the header is a global value in the allocator
 */
static struct AllocHeader *header = NULL;

/*
 * Macros for addressing into the bins.
 * Fast bins are sized to exact 8 byte splits up to 512.
 * Range bins can actually address fast bins, and subdivide then
 * entire chunk space.
 */
#define getRecentBin() (header->bins[0].dirty)
#define getLastSplitRem() (header->bins[0].clean)
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
struct ChunkHeader* initChunk(void *mem, size_t size) {
  assert(IS_WORD_ALIGNED(mem) && "Attempt to initialize chunk which does not align to WORD bound");
  assert(IS_WORD_ALIGNED(size) && "Attempt to initialize chunk whose size will misalign successive chunks");
  assert((size >= MIN_ALLOC_BYTES) && "Attempt to initialize chunk of diminutive size");
  assert(!((char*)mem + size > ramHighAddress) && "Attempt to initialize chunk larger than RAM");
  struct ChunkHeader *chunk = (struct ChunkHeader*)mem;
  writeSize(getTag(chunk), size);
  writeSizePtr(getFooter(chunk), size);
  getTag(chunk).free = 1;
  getFooter(chunk)->free = 1;
  chunk->prev = 0;
  chunk->next = 0;
  return chunk;
}

/*
 * insertChunkBefore :: ChunkHeader -> ChunkHeader -> ()
 *
 * Inserts 'this' before 'that'
 */
void insertChunkBefore(struct ChunkHeader *that, struct ChunkHeader *this) {
  assert(that->prev && "Cannot insert when 'that' node isn't in a list");
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
static void insertChunkAfter(struct ChunkHeader *that, struct ChunkHeader *this) {
  this->next = that->next;
  if (this->next)
    this->next->prev = &this->next;
  that->next = this;
  this->prev = &that->next;
}

/*
 * removeChunk :: ChunkHeader -> ChunkHeader
 *
 * Removes 'this' from its list and returns it.
 */
struct ChunkHeader* removeChunk(struct ChunkHeader *this) {
  *(this->prev) = this->next;
  if (this->next)
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
static void binChunk(struct ChunkHeader *chunk, BinChunkMode mode) {
  struct ChunkBin *bin;
  struct ChunkHeader *chunks = NULL;

  assert(isUnlinked(chunk) && "Chunk has not been unlinked prior to bining");

  bin = &getBin(getSize(chunk));
  switch(mode) {
  case BinDirty:
    if (!(chunks = bin->dirty)) {
      bin->dirty = chunk;
      chunk->prev = &bin->dirty;
      return;
    }
    break;
  case BinClean:
    if (!(chunks = bin->clean)) {
      bin->clean = chunk;
      chunk->prev = &bin->clean;
      return;
    }
    break;
  case BinRecent:
    if (!(chunks = getRecentBin())) {
      getRecentBin() = chunk;
      chunk->prev = &getRecentBin();
    } else {
      insertChunkBefore(chunks, chunk);
    }
    return;
  case BinLastSplitRem:
	if (!(chunks = getLastSplitRem())) {
      getLastSplitRem() = chunk;
      chunk->prev = &getLastSplitRem();
	} else {
	  insertChunkBefore(chunks, chunk);
	}
    return;
  }

  while (chunks) {
    if ((intptr_t)chunk < (intptr_t)chunks) {
      insertChunkBefore(chunks, chunk);
      break;
    } else if (!chunks->next) {
      insertChunkAfter(chunks, chunk);
      break;
    } else {
      chunks = chunks->next;
    }
  }

  return;
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
    ram0 = (char*)&_DRAM;
    ramHighAddress = ram0 + HEAP_SIZE;

    /* Zero out the header RAM */
    char *z = ram0;
    for (size_t i = 0; i < sizeof *header; i++) {
      *z++ = 0;
    }
    
    /* Overlay the header at the base of ram */
    header = (struct AllocHeader*)ram0;

    /* align things for the heap. Since a chunk has a WORD sized tag at boths ends
     * and our allocator is required to return pointers which align on DOUBLE WORD
     * boundaries we need to push the start of the heap to a WORD aligned address
     * so that the first chunk user data will be on a DOUBLE WORD address.
     */
#if defined PLATFORM_K70CW
    heap = ram0 + WORD_PAD(sizeof(struct AllocHeader)) + WORD_BYTES;
#else
    heap = ram0 + WORD_PAD(sizeof(struct AllocHeader));
#endif
    
    /* invalidate the heap */
    for (char *x = heap; x < ramHighAddress; x++) {
      *x = (char)0xfa;
    }
    numChunkOffsets = (ramHighAddress - heap) / MIN_ALLOC_BYTES;

    struct ChunkHeader *firstChunk = initChunk(heap, ramHighAddress - heap);
    getBinByIndex(MAX_BINS - 1).clean = firstChunk;
    firstChunk->prev = &getBinByIndex(MAX_BINS - 1).clean;
  }
  return;
}

/*
 * unlinkChunk :: [ChunkHeader] -> [ChunkHeader]
 *
 * Removes the chunk from its linked list.
 * Patches up the prev and next chunks.
 */
static struct ChunkHeader* unlinkChunk(struct ChunkHeader *chunk) {
  if (chunk->prev || chunk->next) {
    chunk = removeChunk(chunk);
    chunk->prev = NULL;
    chunk->next = NULL;
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
static void coalesce(struct ChunkHeader *chunks) {
  struct ChunkHeader *chunk = chunks;

  while (chunk) {
    struct ChunkHeader *next = chunk->next;

    /* Case 1: both pred and succ are free */
    if (getTagPred(chunk)->free && getTagSucc(chunk)->free) {
      struct ChunkHeader *pred = getPred(chunk);
      struct ChunkHeader *succ = getSucc(chunk);
      assert(getTag(pred).free && "Tag ismatch in pred coalesce(1)");
      assert(getTag(succ).free && "Tag mismatch in succ coalesce(1)");
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
      struct ChunkHeader *pred = getPred(chunk);
      assert(getTag(pred).free && "Tag mismatch in pred coalesce(2)");
      writeSize(getTag(pred), getSize(pred) + getSize(chunk));
      writeSizePtr(getFooter(chunk), getSize(pred));
      unlinkChunk(chunk);
      zeroFooter(pred);
      zeroTag(chunk);
      binChunk(unlinkChunk(pred), BinClean);
    } else if (getTagSucc(chunk)->free) { /* Case 3: only succ is feee */
      struct ChunkHeader *succ = getSucc(chunk);
      assert(getTag(succ).free && "Tag mismatch in succ coalesce(3)");
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
static struct ChunkHeader* exactFitSearch(struct ChunkHeader *chunks, size_t size, ExactFitRebinFlag rebinMismatch) {
  struct ChunkHeader *chunk = chunks;

  while (chunk) {
    if (isExactMatch(chunk, size)) {
      return chunk;
    }
    if (rebinMismatch) {
      struct ChunkHeader *next = chunk->next;
      binChunk(unlinkChunk(chunk), BinDirty);
      chunk = next;
    } else {
      chunk = chunk->next;
    }
  }
  return NULL;
}

/*
 * firstFitSearch :: [ChunkHeader] -> Integer -> Maybe ChunkHeader
 *
 * Does a first fit search though the chunk list and returns
 * a match, or NULL when search fails.
 */
static struct ChunkHeader* firstFitSearch(struct ChunkHeader *chunks, size_t size) {
  struct ChunkHeader *chunk = chunks;

  while (chunk) {
    if (getSize(chunk) >= size) {
      return chunk;
    }
    chunk = chunk->next;
  }
  return NULL;
}

/*
 * splitChunk :: ChunkHeader -> Integer -> ChunkHeaderPtr -> ChunkHeader
 *
 * Cleaves a chunk of size 'size' from 'chunk' and another chunk 'rest'
 * which is the remainder. 'rest' is returned via an out pointer.
 * the new chunk is returned.
 */
struct ChunkHeader* splitChunk(struct ChunkHeader* chunk, size_t size, struct ChunkHeader **rest) {
  assert((getSize(chunk) > size) && "Something is wrong. Splitting from too small a chunk");
  assert(IS_WORD_ALIGNED(size) && "Something is wrong. Size is not WORD aligned");
  assert(IS_WORD_ALIGNED(getSize(chunk)) && "Something is wrong. Chunk is not WORD aligned");

  size_t oldSize = getSize(chunk);
  struct ChunkHeader *chunkA = initChunk(chunk, size);
  struct ChunkHeader *chunkB = initChunk((char*)chunk + size, oldSize - size);

  assert(IS_WORD_ALIGNED(chunkA) && "Split chunk A is not on a WORD boundary");
  assert(IS_WORD_ALIGNED(chunkB) && "Split chunk B is not on a WORD boundary");
  assert((getSize(chunkA) == size) && "Something is wrong. Split chunk A size incorrect");
  assert((readSizePtr(getFooter(chunkA)) == size) && "Something is wrong. Split chunk A footer size incorrect");
  assert((getSize(chunkB) == (oldSize - size)) &&"Something is wrong. Split chunk B size incorrect");
  assert((readSizePtr(getFooter(chunkB)) == (oldSize - size)) && "Something is wrong. Split chunk B size incorrect");
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
static struct ChunkHeader* allocateChunk(size_t size) {
  struct ChunkHeader *chunk = NULL;
  struct ChunkHeader *rest = NULL;

  /* Step 1: First check if the last free'd block is suitable
   *         does not exceed the requested size by no more than
   *         MIN_ALLOC_BYTES
   */
  if (isExactMatch(getRecentBin(), size)) {
    chunk = unlinkChunk(getRecentBin());
    goto exit;
  }

  /* Step 2: Compute this chunks bin and locate an exact match if possible
   *
   * Use this time to coalesce too.
   */
  if ((chunk = exactFitSearch(getBin(size).dirty, size, ExactFitDontRebin))) {
    chunk = unlinkChunk(chunk);
    goto exit;
  }

  /* Only coalesce if there are dirty chunks, but no match */
  if (getBin(size).dirty) {
    coalesce(getBin(size).dirty);
    struct ChunkHeader *rb = getRecentBin();
    coalesce(rb);
  }

  /* Step 3: See if there is an exact chunk anywhere in the recent bin.
   *         Failed matches get pushed onto a dirty bin list of the correct size
   */
  if ((chunk = exactFitSearch(getRecentBin(), size, ExactFitDoRebin))) {
    chunk = unlinkChunk(chunk);
    goto exit;
  }

  /* Step 4: Check if the last split produced a remainder which can be used
   *         to satisfy this allocation
   */
  if ((chunk = firstFitSearch(getLastSplitRem(), size))) {
    chunk = unlinkChunk(chunk);
    goto split;
  }

  /* Step 5: Does the bin have a chunk in its clean list */
  if ((chunk = firstFitSearch(getBin(size).clean, size))) {
    chunk = unlinkChunk(chunk);
    goto split;
  }

  /* Step 6: Scan all larger bins for a chunk ... */
  for (int i = getBinIndex(size) + 1; i < MAX_BINS; i++) {
    if ((chunk = firstFitSearch(getBinByIndex(i).clean, size))) {
      chunk = unlinkChunk(chunk);
      goto split;
    }
  }

  /* Step 7: Coalesce space until a fit is found */
  for (int i = getBinIndex(size) + 1; i < MAX_BINS; i++) {
	coalesce(getBinByIndex(i).dirty);
    if ((chunk = firstFitSearch(getBinByIndex(i).clean, size))) {
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
      struct ChunkHeader *pre = splitChunk(rest, size, &rest);
      binChunk(pre, BinRecent);
      sizeRest = getSize(rest);
    }
  }

 binChunk(rest, BinLastSplitRem);

exit:

  return chunk;
}

/*
 * malloc :: Integer -> Ptr
 *
 * allocate chunk of at least 'size' size.
 * Returned pointer will be double word aligned.
 * If size is 0, a chunk of MIN_ALLOC_BYTES is returned.
 */
void* malloc(size_t size) {
  size_t newSize = size + (2 * sizeof(struct ChunkTag));

  initRam();
  
  if (newSize < MIN_ALLOC_BYTES) {
    newSize += (MIN_ALLOC_BYTES - newSize);
  }

  struct ChunkHeader *chunk = allocateChunk(DWORD_PAD(newSize));

  if (chunk) {
    void *mem = getPayload(chunk);

    header->lastAllocSize = getSize(chunk);

    /* store the PID of the caller in the previously reserved byte just before the footer */
    getTag(chunk).pid = getFooter(chunk)->pid = getCurrentPID();

    /* indicate this chunk is allocated */
    getTag(chunk).free = getFooter(chunk)->free = 0;

    /* tag this address as allocated in the bitmap */
    assert(!checkBitmap(mem) && "Memory error. Cannot allocate adress already allocated.");
    setBitmap(mem);
    return (void*)mem;
  } else {
    return NULL;
  }
}

/*
 * mallocz :: Integer -> Ptr
 *
 * allocate chunk via malloc and zero the memory
 */
void* mallocz(size_t size) {
  char *mem = malloc(size);

  if (mem) {
    for (size_t i = 0; i < size; i++) {
      *mem++ = (char)0;
    }
  }

  return mem;
}

/*
 * free :: Ptr -> ()
 *
 * free allocated chunk. If ptr is NULL or not from malloc, this is a noop.
 */
void free(void *ptr) {
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
    assert(checkBitmap(ptr) && "Memory error. Cannot free address not allocated by malloc");
   
    /*
     * Safetey check #3: We only allocate addresses between heap and ramHighAddress
     */
    assert(((char*)ptr >= heap) && ((char*)ptr <= ramHighAddress) && "Memory error. Address out of allocator zone");
    if (checkBitmap(ptr)) {
      struct ChunkHeader *chunk = (struct ChunkHeader*)((uintptr_t)ptr - sizeof(struct ChunkTag));
      getTag(chunk).free = 1;
      getFooter(chunk)->free = 1;
      chunk->next = NULL;
      chunk->prev = NULL;
      clearBitmap(ptr);
      binChunk(chunk, BinRecent);
    }
  }
}

/*
 * hexdump :: Ptr -> Integer -> FILE* -> ()
 *
 * Hexdump a buffer to 'out'
 */
void hexdump(FILE* out, void *buf, size_t length) {
  char ascii[33] = {0};
  char *p = (char*)buf;
  int i;
  fflush(out);
  for (i = 0; i < length; i++) {
    if (!(i % 32)) {
      if (i != 0) {
        printf("  %s\n", ascii);
      }

      printf(" %.8lx ", (uintptr_t)heap + i);
    }

    printf(" %.2x", (uint8_t)p[i]);

    if ((p[i] >= 0x20) && (p[i] <= 0x7e)) {
      ascii[i % 32] = p[i];
    } else {
      ascii[i % 32] = '.';
    }
    ascii[(i % 32) + 1] = '\0';
  }

  while ((i++ % 32) != 0) {
    printf("  ");
  }

  printf(" %s\n", ascii);
  fflush(stdout);
  UNUSED(out);
}

/*
 * dumpChunk :: FILE* -> ChunkHeader -> Bool -> ()
 *
 * Dumps a chunk to out
 */
void dumpChunk(FILE *out, struct ChunkHeader* chunk, int doHexdump) {
  printf("**** YAMalloc Chunk Dump ****\n\n");
  printf("General Info:\n\n");
  printf("    Chunk addr: 0x%08lx\n", (uintptr_t)chunk);
  
  uintptr_t mem = (uintptr_t)getPayload(chunk);
  printf("    Chunk user addr: 0x%08lx\n", mem);
  printf("    Chunk bitmap addreds (off: %d,byte: %d,bit: %d)\n", getAddrBitmapOffset(mem), getAddrByte(mem), getAddrBit(mem));
  printf("    Chunk user addr DWORD aligned?: %s\n", IS_DWORD_ALIGNED(getPayload(chunk)) ? "yes" : "no");
  
  uint32_t st = readSize(getTag(chunk)), sf = readSizePtr(getFooter(chunk));
  printf("    Chunk size: %ld (%ld)%s\n", st, sf, st == sf ? "" : " MISMATCH!");
  
  int ft = getTag(chunk).free, ff = getFooter(chunk)->free;
  printf("    Chunk isFree: %d (%d)%s\n", ft, ff, ft == ff ? "" : " MISMATCH!");
  
  printf("    Sanity Checks. Pred Size: %ld, Free: %d\n", readSizePtr(getTagPred(chunk)), getTagPred(chunk)->free);
  printf("                   Succ Size: %ld, Free: %d\n", readSizePtr(getTagSucc(chunk)), getTagSucc(chunk)->free);

  int isFree = getTag(chunk).free;

  if (!isFree) {
	if (!checkBitmap(mem)) {
	  printf("    WARNING: Allocated chunk NOT recorded in BITMAP!\n");
	}
    printf("    Chunk PID: %d (%d)\n", getTag(chunk).pid, getFooter(chunk)->pid);
  } else {
	if (checkBitmap(mem)){
	  printf("    WARNING: Free chunk HAS record in BITMAP!\n");
	}
    printf("    Chunk Prev Ptr: 0x%08lx\n", (uintptr_t)chunk->prev);
    printf("    Chunk Next Ptr: 0x%08lx\n", (uintptr_t)chunk->next);
  }
  
  fflush(stdout);

  if (doHexdump) {
    printf("Memory Dump:\n\n");
    hexdump(out, (void*)chunk, getSize(chunk));
  }
}

/*
 * pprintMem :: FILE* -> ()
 *
 * Dumps the current state of the allocator and its data structures to 'out'
 */
void pprintMem(FILE *out) {
  initRam(); /* incase we haven't initialized the memory already */

  printf("**** YAMalloc Memory Dump ****\n\n");
  printf("General Info:\n\n");
  printf("    Addr ram0:    0x%08lx\n", (uintptr_t)ram0);
  printf("    Addr ramHigh: 0x%08lx\n", (uintptr_t)ramHighAddress);
  printf("    Min. Allocation size (B): %d\n", MIN_ALLOC_BYTES);
  printf("\n");
  fflush(stdout);
  printf("Allocator Header Info:\n\n");
  printf("    # Chunk Offsets In Bitmap: %d\n", numChunkOffsets);
  printf("    Size of bin area (B) : %d\n", sizeof(header->bins[0]) * MAX_BINS);
  printf("    Size of Bitmap (B)   : %d\n", ALLOCATION_BITMAP_SIZE);
  printf("    Size of Header (B)   : %d\n", sizeof(struct AllocHeader));
  printf("    Addr of header (should be ram0): 0x%08lx\n", (uintptr_t)header);
  printf("    Addr of bin 0                  : 0x%08lx\n", (uintptr_t)header->bins);
  printf("    Addr of bin 127                : 0x%08lx\n", (uintptr_t)header->bins + MAX_BINS);
  printf("    Addr of bitmap start           : 0x%08lx\n", (uintptr_t)header->bitmap);
  printf("    Addr of bitmap end             : 0x%08lx\n", (uintptr_t)header->bitmap + ALLOCATION_BITMAP_SIZE);
  printf("    Addr of header end             : 0x%08lx\n", (uintptr_t)((char*)header + sizeof(struct AllocHeader)));
  printf("    Addr of heap start : 0x%08lx\n", (uintptr_t)heap);
  printf("    Size of heap (B)   : %ld\n", (uintptr_t)(ramHighAddress - heap));
  printf("    Last address is DWORD aligned : %s\n", (IS_DWORD_ALIGNED(ramHighAddress) ? "yes" : "no"));
  printf("\n");
  fflush(stdout);
  printf("Bitmap Info:\n\n");

  for (int i = 0; i < ALLOCATION_BITMAP_SIZE; i++) {
    if (!(i % 10)) {
      if (i != 0) {
        printf("\n");
      }

      printf(" %.8lx ", (uintptr_t)header->bitmap + i);
    } else if (i != 0) {
      printf(" ");
    }

    char c = header->bitmap[i];
    for (int j = 8; j > 0; j--) {
      fputc('.' + (3 * ((c >> (j - 1)) & 1)), out); /* unset print '.', set print '1' (hence the multiple of 3) */
    }
  }
  printf("\n");

  printf("Heap Info:\n\n");

  for (uintptr_t i = (uintptr_t)heap; i < (uintptr_t)ramHighAddress; ) {
    printf("** Chunk Offset 0x%08lx\n", i);
    struct ChunkHeader *chunk = (struct ChunkHeader*)i;

    if (getSize(chunk) == 0)
      break;

    if ((i + getSize(chunk)) > (uintptr_t)ramHighAddress) {
      printf("** Chunk has potentially corrupt size of %ld\n", getSize(chunk));
      break;
    }

    dumpChunk(out, chunk, 0);
    i += getSize(chunk);
  }

  printf("\n");
  printf("Bin Info:\n\n");

  for (int i = 0; i < MAX_BINS; i++) {
    if (i == 0) {
      struct ChunkHeader *chunks = getRecentBin();
      for (int j = 0; chunks; j++) {
        if (j == 0) {
          printf("** SPECIAL BIN: Recent Chunks\n\n");
        }
        printf("**** Recent Chunks [%d] ****\n\n", j);
        dumpChunk(out, chunks, 0);
        chunks = chunks->next;
      }

      chunks = getLastSplitRem();
      for (int j = 0; chunks; j++) {
        if (j == 0) {
          printf("** SPECIAL BIN: Last Split Remainders\n\n");
        }
        printf("**** Last Split Remainder Chunk [%d] ****\n\n", j);
        dumpChunk(out, chunks, 0);
        chunks = chunks->next; 
      }
    } else {
      struct ChunkHeader *chunks = getBinByIndex(i).dirty;
      for (int j = 0; chunks; j++) {
        if (j == 0) {
          printf("** DIRTY BIN %d\n\n", i);
        }
        printf("**** Bin %d Chunk [%d] ****\n\n", i, j);
        dumpChunk(out, chunks, 0);
        chunks = chunks->next;
      }

      chunks = getBinByIndex(i).clean;
      for (int j = 0; chunks; j++) {
        if (j == 0) {
          printf("** CLEAN BIN %d\n\n", i);
        }
        printf("**** Bin %d Chunk [%d] ****\n\n", i, j);
        dumpChunk(out, chunks, 0);
        chunks = chunks->next;
      }
    }
  }
}
