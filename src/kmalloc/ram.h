#define ONE_MEBIBYTE 1048576 /* 1024 * 1024 */
#define ONE_MEBIBYTE_WITH_DWORD_PADDING 1048583 /* ONE_MEBIBYTE + 7 bytes */
static char _DRAM[ONE_MEBIBYTE_WITH_DWORD_PADDING];

#define SDRAM_START (char*)&_DRAM
#define SDRAM_SIZE ONE_MEBIBYTE
#define SDRAM_END (SDRAM_START + SDRAM_SIZE)

/**
 * This can be computed at runtime using the following method
 *
 * Let M be the size of the addressable memory in bytes.
 * Let k be the size of the minimum addressable chunk.
 * Let h be the size of the allocators header.
 * Let a byte hold 8 bits.
 * Solve m = 1/(8k + 1) * 8k(M - h)
 * Solve b = m / 8k, ALLOCATION_BITMAP_SIZE = b
 */
#define ALLOCATION_BITMAP_SIZE 8128 /* M = ONE_MEBIBYTE, k = MIN_ALLOC_BYTES, h = 1028 */

/*
 * Note that this header assumes that MIN_ALLOC_BYTES == 16, and the
 * value of ALLOCATION_BITMAP_SIZE was computed with that assumption.
 * This is dangerous, if MIN_ALLOC_BYTES ever changes, and it might,
 * since it is equal to the sizeof struct ChunkHeader.
 *
 * (Note too it assumes MIN_ALLOC_BYTES will be defined for us)
 *
 * So below is a bit of C trickery gleaned from the internet
 * (Here: http://scaryreasoner.wordpress.com/2009/02/28/checking-sizeof-at-compile-time/)
 * (And more trickery here: http://www.pixelbeat.org/programming/gcc/static_assert.html)
 *
 * this code prevents compilation if the size assumption changes.
 */

#ifndef MIN_ALLOC_BYTES
#error include ram.h after MIN_ALLOC_BYTES is defined
#endif

/* ASSERT MIN_ALLOC_BYTES == 16 */
#define assert_min_alloc_bytes(e) enum { assert_mab = sizeof(char[1 - 2*!(e)]) }
assert_min_alloc_bytes(MIN_ALLOC_BYTES == 16);

/* ASSERT ALLOCATION_BITMAP_SIZE is multiple of DWORD_BYTES */
#define assert_allocation_bitmap_size(e) enum { assert_abs = sizeof(char[1 - 2*!(e)]) }
assert_allocation_bitmap_size(ALLOCATION_BITMAP_SIZE % DWORD_BYTES == 0);
