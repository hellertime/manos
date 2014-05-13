#include <manos.h>

int syspostsignal(Pid pid, ProcSig signal) {
    Proc* p;
    enterCriticalRegion();
    p = procTable[pid];
    p->sigPending |= signal & ~p->sigMask;
    leaveCriticalRegion();
    return 0;
}
