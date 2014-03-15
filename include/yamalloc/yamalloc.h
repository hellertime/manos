#ifndef YAMALLOC_H
#define YAMALLOC_H

#include <stdio.h>

void* yamalloc(size_t size);
void yafree(void* ptr);
void yadump(FILE *out);

#endif /* ! YAMALLOC_H */
