#ifndef MANOS_LIBC_H
#define MANOS_LIBC_H

#define COUNT_OF(x) (sizeof(x) / sizeof(0[x]))
#define UNUSED(x) (void)(x)

void* malloc(size_t size);
void* mallocz(size_t size);
void free(void *ptr);

#endif /* ! MANOS_LIBC_H */
