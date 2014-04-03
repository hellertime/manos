#ifndef MANOS_ARCH_MK70F12_H
#define MANOS_ARCH_MK70F12_H

#ifdef PLATFORM_K70CW

#define ENODATA EINVAL
#define ENOTRECOVERABLE EINVAL

#define PRIu32 "lu"
#define PRIx32 "lx"
#define PRIuPTR PRIu32
#define PRIxPTR PRIx32

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

#define fprintf(x,y,...) printf(y,__VA_ARGS__)

#endif /* PLATFORM_K70CW */

#endif /* ! MANOS_ARCH_MK70F12_H */
