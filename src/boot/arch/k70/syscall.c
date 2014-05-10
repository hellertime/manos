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
int __attribute__((naked)) __attribute__((noinline)) kexec(const char* path, char * const argv[]) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    : 
    : [syscall] "I" (MANOS_SYSCALL_EXEC)
);
}
#pragma GCC diagnostic pop
#else
int kexec(const char* path, char * const argv[]) {
    return sysexecv(path, argv);
}
#endif

/*
 * File Management System Calls
 */

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void __attribute__((naked)) __attribute__((noinline)) kclose(int fd) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_CLOSE)
);
}
#pragma GCC diagnostic pop
#else
void kclose(int fd) {
    sysclose(fd);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
int __attribute__((naked)) __attribute__((noinline)) kfstat(int fd, NodeInfo* ni) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_FSTAT)
);
}
#pragma GCC diagnostic pop
#else
int kfstat(int fd, NodeInfo* ni) {
    return sysgetInfoFd(fd, ni);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
int __attribute__((naked)) __attribute__((noinline)) kopen(const char* path, Caps caps) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_OPEN)
);
}
#pragma GCC diagnostic pop
#else
int kopen(const char* path, Caps caps) {
    return sysopen(path, caps);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
ptrdiff_t __attribute__((naked)) __attribute__((noinline)) kread(int fd, void* buf, size_t n) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_READ)
);
}
#pragma GCC diagnostic pop
#else
ptrdiff_t kread(int fd, void* buf, size_t n) {
    return sysread(fd, buf, n);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
ptrdiff_t __attribute__((naked)) __attribute__((noinline)) kwrite(int fd, void* buf, size_t n) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_WRITE)
);
}
#pragma GCC diagnostic pop
#else
ptrdiff_t kwrite(int fd, void* buf, size_t n) {
    return syswrite(fd, buf, n);
}
#endif

/**
 * IPC system calls
 */

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
int __attribute__((naked)) __attribute__((noinline)) trylock(Lock* l) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_TRYLOCK)
);
}
#pragma GCC diagnostic pop
#else
int trylock(Lock* l) {
    return systrylock(l);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wreturn-type"
#pragma GCC diagnostic ignored "-Wunused-parameter"
void __attribute__((naked)) __attribute__((noinline)) lock(Lock* l) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_LOCK)
);
}
#pragma GCC diagnostic pop
#else
void lock(Lock* l) {
    return syslock(l);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void __attribute__((naked)) __attribute__((noinline)) unlock(Lock* l) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_UNLOCK)
);
}
#pragma GCC diagnostic pop
#else
void unlock(Lock* l) {
    sysunlock(l);
}
#endif

#ifdef PLATFORM_K70CW
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void __attribute__((naked, noinline)) waitpid(int pid) {
__asm(
    "svc %[syscall]\n\t"
    "bx lr"
    :
    : [syscall] "I" (MANOS_SYSCALL_WAITPID)
    );
}
#pragma GCC diagnostic pop
#else
void waitpid(int pid) {
    syswaitpid(pid);
}
#endif

#else
#error "Unsupported Compiler"
#endif
