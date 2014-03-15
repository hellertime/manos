#ifndef SHELL_MEM_H
#define SHELL_MEM_H

/**
 * mem.h
 * Shell memory routines
 */

#include <stddef.h>

#define malloc(x) shellMalloc(x)
#define free(x) shellFree(x)

void* shellMalloc(size_t size);
void shellFree(void *ptr);

#define memcpy(a, b, n) shellMemcpy(a, b, n)

void* shellMemcpy(void *a, const void *b, size_t n);

#endif /* ! SHELL_MEM_H */
