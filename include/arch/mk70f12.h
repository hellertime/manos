#ifndef MANOS_ARCH_MK70F12_H
#define MANOS_ARCH_MK70F12_H

#ifdef PLATFORM_K70CW

#define ENOBUFS EINVAL
#define ENODATA EINVAL
#define ENOTRECOVERABLE EINVAL

#define PRIu32 "lu"
#define PRIx32 "lx"
#define PRIuPTR PRIu32
#define PRIxPTR PRIx32

typedef int32_t intptr_t;
typedef uint32_t uintptr_t;

void mcgInit(void);
void sdramInit(void);

#endif /* PLATFORM_K70CW */

#endif /* ! MANOS_ARCH_MK70F12_H */
