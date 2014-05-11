#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>

void syswaitpid(int pid) {
    Proc* p;
    enterCriticalRegion();
    LIST_FOR_EACH_ENTRY(p, &procRunQ, nextRunQ) {
        if (p->pid == pid) {
            ASSERT(listIsEmpty(&rp->nextWaitQ) && "syswaitpid() running process already waiting");
            listAddAfter(&rp->nextWaitQ, &p->waitQ);
            rp->state = ProcWaiting;
            YIELD();
            break;
        }
    }
    leaveCriticalRegion();
}
