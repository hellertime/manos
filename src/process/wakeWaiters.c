#include <manos.h>
#include <manos/list.h>

void wakeWaiting(Proc* p) {
    Proc* waiting;
    Proc* save;

    LIST_FOR_EACH_ENTRY(waiting, save, &p->waitQ, nextWaitQ) {
        listUnlinkAndInit(&waiting->nextWaitQ);
        waiting->state = ProcReady;
    }
}
