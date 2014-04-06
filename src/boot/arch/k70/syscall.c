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
#pragma GCC diagnostic ignored "-Wunused-parameter"
int __attribute__((naked)) __attribute__((noinline)) exec(const char* path, char * const argv[]) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    : 
    : [syscall] "I" (MANOS_SYSCALL_EXEC)
);
}
#pragma GCC diagnostic pop
#else
int exec(const char* path, char * const argv[]) {
    return sysexecv(path, argv);
}
#endif

/*
 * File Management System Calls
 */

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void __attribute__((naked)) __attribute__((noinline)) close(int fd) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_CLOSE)
);
}
#pragma GCC diagnostic pop
#else
void close(int fd) {
    sysclose(fd);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
int __attribute__((naked)) __attribute__((noinline)) fstat(int fd, NodeInfo* ni) {
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
#pragma GCC diagnostic ignored "-Wunused-parameter"
int __attribute__((naked)) __attribute__((noinline)) open(const char* path, Caps caps) {
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
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
ptrdiff_t __attribute__((naked)) __attribute__((noinline)) read(int fd, void* buf, size_t n) {
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
