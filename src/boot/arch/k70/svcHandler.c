#include <manos.h>

#ifdef PLATFORM_K70CW

#ifdef __GNUC__
void __attribute__((naked)) svcHandler(void) {
__asm(
   "tst    lr, #4\n\t"
   "ite    eq\n\t"
   "mrseq  r0, msp\n\t"
   "mrsne  r0, psp\n\t"
   "push   {lr}\n\t"
   "bl     svcHandlerDispatch\n\t"
   "pop    {pc}"
);
}
#else
#error "Unsupported Compiler"
#endif

static int execSyscall(int* args) {
    return sysexecv((const char*)args[0], (char * const *)args[1]);
}

static void closeSyscall(int* args) {
    sysclose(args[0]);
}

static int fstatSyscall(int* args) {
    return sysgetInfoFd(args[0], (NodeInfo*)args[1]);
}

static int openSyscall(int* args) {
    return sysopen((const char*)args[0], (Caps)args[1]);
}

static int readSyscall(int* args) {
    return sysread(args[0], (void*)args[1], (size_t)args[2]);
}

#include <arch/k70/syscall.x>

#include "syscall.h"

#define X(c, f, r) { .fn = f##Syscall, .isVoid = r },
static struct {
    int (*fn)(int*);
    int isVoid;
} dispatchTable[] = {
    SYSCALL_MAP
};
#undef X

#define X(c, f, r) case MANOS_SYSCALL_##c:
void svnHandlerDispatch(StackFrame* frame) {
    SyscallIndex idx = (SyscallIndex)((uint8_t*)frame->ret - 2);
    switch(idx) {
    SYSCALL_MAP
        if (dispatchTable[idx].isVoid)
            dispatchTable[idx].fn(frame->a);
        else
            frame->ret = dispatchTable[idx].fn(frame->a);
       break;
    default:
        sysprintln("Uknown SVN: %d", idx);
    }
}
#undef X

#endif