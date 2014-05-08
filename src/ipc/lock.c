#include <assert.h>
#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>

/**
 * trylock() - obtains a lock or returns false
 * @l:         lock to obtain
 */
int systrylock(Lock* l) {
    int haveLock = 0;

    if (!rp)
        return 1;

    DISABLE_INTERRUPTS();
    if (! l->locked) {
        l->locked = 1;
        haveLock = 1;
    }
    ENABLE_INTERRUPTS();
    return haveLock;
}

void syslock(Lock* l) {
#ifdef PLATFORM_K70CW
    if (!rp)
        return;

    DISABLE_INTERRUPTS();
    while (l->locked) {
        assert(listIsEmpty(rp->nextWaitQ) && "lock() running process already waiting on something else!");
        listAddBefore(&l->q, &rp->nextWaitQ);
        rp->state = ProcWaiting;
        YIELD();
        ENABLE_INTERRUPTS();
        DISABLE_INTERRUPTS();
    }
    l->locked = 1;
    ENABLE_INTERRUPTS();
#endif
}

void sysunlock(Lock* l) {
    if (!rp)
        return;

    DISABLE_INTERRUPTS();
    l->locked = 0;
    if (! listIsEmpty(&l->q)) {
        Proc* p = CONTAINER_OF(&l->q, Proc, nextWaitQ);
        listUnlink(&l->q);
        p->state = ProcReady;
    }
    ENABLE_INTERRUPTS();
}
