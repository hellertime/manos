/**
 * mem.c
 * Shell memory routines
 *
 * Currently just wraps the C stdlib memory routines when possible.
 */

#ifdef USE_YAMALLOC
#include <libc.h>
#else
#include <stdlib.h>
#endif

/*
 * shellMalloc :: Int -> Mem
 *
 * Wraps the system malloc
 */
void* shellMalloc(size_t size) {
  return malloc(size);
}

/*
 * shellFree :: Mem -> ()
 *
 * Wraps the system free
 */
void shellFree(void *ptr) {
  free(ptr);
}

/*
 * shellMemcpy :: Mem -> Mem -> Int -> ()
 *
 * Simple memory copy
 */
void* shellMemcpy(void *src, const void *dst, size_t n) {
  char *cSrc = src;
  const char *cDst = dst;

  while(n-- > 0) {
    *cSrc++ = *cDst++;
  }

  return src;
}
