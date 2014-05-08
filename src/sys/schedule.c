#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>

/**
 * nextRunnableProc() - return a proc to run to the caller
 *
 * This does not return until a process is ready to run
 */
Proc* nextRunnableProc(void) {
    Proc* p;

    DISABLE_INTERRUPTS();
    if (listIsEmpty(&procRunQ)) {
        p = rp; /* give that man another quantum */
    } else {
        p = CONTAINER_OF((&procRunQ)->next, Proc, nextRunQ);
        listUnlinkAndInit((&procRunQ)->next);
        p->state = ProcReady;
    }
    ENABLE_INTERRUPTS();
    return p;
}

/**
 * scheduleProc() - this is an arch independent scheduler routine
 *
 * Quatum interrupt calls into this, but this call doesn't return
 */
uint32_t __attribute__((used)) scheduleProc(uint32_t sp) {
    STOP_SYSTICK();
    if (rp) rp->sp = sp;
    Proc* p = nextRunnableProc();
    rp = p;
    rp->state = ProcRunning;
    RESET_SYSTICK();
    START_SYSTICK();
    return rp->sp;
}
