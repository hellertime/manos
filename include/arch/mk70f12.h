#ifndef MANOS_ARCH_MK70F12_H
#define MANOS_ARCH_MK70F12_H

#ifdef PLATFORM_K70CW

#ifndef ENOBUFS
#define ENOBUFS EINVAL
#endif

#ifndef ENODATA
#define ENODATA EINVAL
#endif

#ifndef ENOTRECOVERABLE
#define ENOTRECOVERABLE EINVAL
#endif

#ifndef PRIu32
#define PRIu32 "lu"
#endif

#ifndef PRIx32
#define PRIx32 "lx"
#endif

#ifndef PRIuPTR
#define PRIuPTR PRIu32
#endif

#ifndef PRIxPTR
#define PRIxPTR PRIx32
#endif

#ifndef intptr_t
typedef int32_t intptr_t;
#endif

#ifndef uintptr_t
typedef uint32_t uintptr_t;
#endif

void mcgInit(void);
void sdramInit(void);

#endif /* PLATFORM_K70CW */

#endif /* ! MANOS_ARCH_MK70F12_H */
