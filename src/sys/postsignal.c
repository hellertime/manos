#include <manos.h>

int syspostsignal(Pid pid, ProcSig signal) {
    if (pid < 0 || pid > MANOS_MAXPROC)
        return 0;

    Proc* p;
    enterCriticalRegion();
    p = procTable[pid];
    p->sigPending |= signal & ~p->sigMask;
    leaveCriticalRegion();
    return 0;
}
