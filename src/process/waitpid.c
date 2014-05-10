#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>

void syswaitpid(int pid) {
    Proc* p;
    DISABLE_INTERRUPTS();
    LIST_FOR_EACH_ENTRY(p, &procRunQ, nextRunQ) {
        if (p->pid == pid) {
            assert(listIsEmpty(&rp->nextWaitQ) && "syswaitpid() running process already waiting");
            addListAfter(&rp->nextWaitQ, &p->waitQ);
            rp->state = ProcWaiting;
            YIELD();
            ENABLE_INTERRUPTS();
            break;
        }
    }
    ENABLE_INTERRUPTS();
}
