#ifndef MANOS_LIBC_H
#define MANOS_LIBC_H

void* malloc(size_t size);
void yafree(void *ptr);
void yadump(...); /* TODO: write to fd? /dev/cons */

#endif /* ! MANOS_LIBC_H */
