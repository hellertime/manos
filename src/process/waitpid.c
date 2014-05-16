#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>

void syswaitpid(int pid) {
    if (pid < 0 || pid > MANOS_MAXPROC)
        return;

    Proc* p;
    enterCriticalRegion();
    p = procTable[pid];
    ASSERT(listIsEmpty(&rp->nextWaitQ) && "syswaitpid() running process already waiting");
    listAddAfter(&rp->nextWaitQ, &p->waitQ);
    rp->state = ProcWaiting;
    YIELD();
    leaveCriticalRegion();
}
