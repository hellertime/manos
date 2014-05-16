#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>

/**
 * trylock() - obtains a lock or returns false
 * @l:         lock to obtain
 */
int systrylock(Lock* l) {
    int haveLock = 0;

    if (!rp || rp->pid == -1)
        return 1;

    enterCriticalRegion();
    if (! l->locked) {
        l->locked = 1;
        haveLock = 1;
        l->pid = rp->pid;
    }
    leaveCriticalRegion();
    return haveLock;
}

void syslock(Lock* l) {
    ASSERT(l && "syslock() NULL Lock");
#ifdef PLATFORM_K70CW
    if (!rp || rp->pid == -1)
        return;

    ASSERT(rp->state != ProcDead && "lock() dead procs can't hold locks");
    enterCriticalRegion();
    while (l->locked) {
        ASSERT(listIsEmpty(&rp->nextWaitQ) && "lock() running process already waiting on something else!");
        listAddBefore(&rp->nextWaitQ, &l->q);
        rp->state = ProcWaiting;
        YIELD();
        leaveCriticalRegion();
        enterCriticalRegion();
    }
    l->locked = 1;
    l->pid    = rp->pid;
    leaveCriticalRegion();
#endif
}

void sysunlock(Lock* l) {
    if (!rp || rp->pid == -1)
        return;

    enterCriticalRegion();
    l->locked = 0;
    l->pid    = -1;
    if (! listIsEmpty(&l->q)) {
        Proc* p = CONTAINER_OF(&l->q, Proc, nextWaitQ);
        listUnlinkAndInit(&l->q);
        p->state = ProcReady;
        YIELD();
    }
    leaveCriticalRegion();
}
