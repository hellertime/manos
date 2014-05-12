#include <manos.h>

int syspostsignal(Pid pid, ProcSig signal) {
    Proc* p;
    enterCriticalRegion();
    p = procTable[pid];
    enqueueHeapQ(p->signalQ, signal);
    p->sigPending = 1;
    leaveCriticalRegion();
    return 0;
}
