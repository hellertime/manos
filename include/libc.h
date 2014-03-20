#ifndef MANOS_LIBC_H
#define MANOS_LIBC_H

#include <stddef.h>

#include <stdio.h>

#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))
#define UNUSED(x) (void)(x)
#define USED(x) (void)(x)

/* malloc.c */
void free(void *ptr);
void* malloc(size_t size);
void* mallocz(size_t size);
void pprintMem(FILE *out);

/* string.c */
char* strdup(const char *c);
int streq(const char *a, const char *b);

#endif /* ! MANOS_LIBC_H */
