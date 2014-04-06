/*
 * K70 System Calls
 *
 * System call structure follows the ARM Procedure Call Standard
 */

#include <manos.h>

#include "syscall.h"

#ifdef __GNUC__

/*
 * Process Management System Calls
 */

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) exec(const char*, char * const []) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    : 
    : [syscall] "I" (MANOS_SYSCALL_EXEC)
);
}
#pragma GCC diagnostic pop
#else
int exec(const char* path, char * const args[]) {
    return sysexecv(path, args);
}
#endif

/*
 * File Management System Calls
 */

#ifdef PLATFORM_K70CW
void __attribute__((naked)) __attribute__((noinline)) close(int) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_CLOSE)
);
}
#else
void close(int fd) {
    sysclose(fd);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) fstat(int, NodeInfo*) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_FSTAT)
);
}
#pragma GCC diagnostic pop
#else
int fstat(int fd, NodeInfo* ni) {
    return sysgetInfoFd(fd, ni);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
int __attribute__((naked)) __attribute__((noinline)) open(const char*, Caps) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_OPEN)
);
}
#pragma GCC diagnostic pop
#else
int open(const char* path, Caps caps) {
    return sysopen(path, caps);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnositc ignored "-Wreturn-type"
ptrdiff_t __attribute__((naked)) __attribute__((noinline)) read(int, void*, size_t) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_READ)
);
}
#pragma GCC diagnostic pop
#else
ptrdiff_t read(int fd, void* buf, size_t n) {
    return sysread(fd, buf, n);
}
#endif

#else
#error "Unsupported Compiler"
#endif
