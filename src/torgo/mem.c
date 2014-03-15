/**
 * mem.c
 * Shell memory routines
 *
 * Currently just wraps the C stdlib memory routines when possible.
 */

#ifdef USE_YAMALLOC
#include <manos/yamalloc.h>
#else
#include <stdlib.h>
#endif

/*
 * shellMalloc :: Int -> Mem
 *
 * Wraps the system malloc
 */
void* shellMalloc(size_t size) {
#ifdef USE_YAMALLOC
  return yamalloc(size);
#else
  return malloc(size);
#endif
}

/*
 * shellFree :: Mem -> ()
 *
 * Wraps the system free
 */
void shellFree(void *ptr) {
#ifdef USE_YAMALLOC
  yafree(ptr);
#else
  free(ptr);
#endif
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
