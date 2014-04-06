#include <manos.h>

#ifdef PLATFORM_K70CW

#ifdef __GNUC__

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

static int writeSyscall(int* args) {
    return syswrite(args[0], (void*)args[1], (size_t)args[2]);
}

#include <arch/k70/syscall.x>

#include "syscall.h"

#define X(c, f, r) { .fn = (int(*)(int*))f##Syscall, .isVoid = r },
static struct {
    union {
        int (*fn)(int*);
        void (*vfn)(int*);
    };
    int isVoid;
} dispatchTable[] = {
    SYSCALL_MAP
};
#undef X

#define X(c, f, r) case MANOS_SYSCALL_##c:
static void __attribute__((used)) svcHandlerDispatch(StackFrame* frame) {
    SyscallIndex idx = (SyscallIndex)(((uint8_t*)frame->ret)[-2]);
    switch(idx) {
    SYSCALL_MAP
        if (dispatchTable[idx].isVoid)
            dispatchTable[idx].vfn(frame->a);
        else
            frame->a[0] = dispatchTable[idx].fn(frame->a);
       break;
    default:
        sysprintln("Uknown SVC: %d", idx);
        break;
    }
    return;
}
#undef X

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


#endif
