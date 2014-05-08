#include <manos.h>
#include <manos/list.h>

/**
 * nextRunnableProc() - return a proc to run to the caller
 *
 * This does not return until a process is ready to run
 */
Proc* nextRunnableProc(void) {
    Proc* p;

    ENABLE_INTERRUPTS();
    while (listIsEmpty(&procRunQ))
        ;
    DISABLE_INTERRUPTS();
    p = CONTAINER_OF((&procRunQ)->next, Proc, nextRunQ);
    listUnlinkAndInit((&procRunQ)->next);
    p->state = ProcReady;
    return p;
}

/**
 * scheduleProc() - this is an arch independent scheduler routine
 *
 * Quatum interrupt calls into this, but this call doesn't return
 */
uint32_t __attribute__((used)) scheduleProc(void) {
    Proc* p = nextRunnableProc();
    rp = p;
    rp->state = ProcRunning;
    return rp->sp;
}
