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
    Proc* save;

    enterCriticalRegion();

    LIST_FOR_EACH_ENTRY_SAFE(p, save, &procRunQ, nextRunQ) {
        if (p->state == ProcDead) {
            listUnlink(&p->nextRunQ);
            recycleProc(p);
        } else if (p->state == ProcReady) {
            listUnlinkAndInit(&p->nextRunQ);
            break;
        }
    }

    if (p->state != ProcReady) {
        p = rp; /* nothing ready, cycle another quantum */
    } else if (rp != NULL) {
        listAddBefore(&rp->nextRunQ, &procRunQ);
    }

    leaveCriticalRegion();
    return p;
}

/**
 * scheduleProc() - this is an arch independent scheduler routine
 *
 * Quatum interrupt calls into this, but this call doesn't return
 */
uint32_t __attribute__((used)) scheduleProc(uint32_t sp) {
    STOP_SYSTICK();
    if (rp) {
        ASSERT(*rp->canary1 == *rp->canary2 && "scheduleProc() old proc canaries are not equal");
        ASSERT(sp < (uintptr_t)rp->canary2 && "scheduleProc() old proc sp below canary");
        if (rp->state == ProcRunning) {
            rp->state = ProcReady;
        }
        rp->sp = sp;
    }
    Proc* p = nextRunnableProc();
    rp = p;
    ASSERT(*rp->canary1 == *rp->canary2 && "scheduleProc() new proc canaries are not equal");
    ASSERT(rp->sp < (uintptr_t)rp->canary2 && "scheduleProc() new proc sp below canary");
    rp->state = ProcRunning;
    RESET_SYSTICK();
    START_SYSTICK();
    return rp->sp;
}
