#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>

static void processSignals(Proc* p) {
    uint32_t newPending = 0; /* allow signals to generate signals */
    if (p->sigPending & SigAbort) {
        /* noop */
    } else if (p->sigPending & SigStop) {
        wakeWaiting(p);
        p->state = ProcStopped;
    } else if (p->sigPending & SigContinue) {
        /* noop */
    }
    p->sigPending = newPending;
}
/**
 * nextRunnableProc() - return a proc to run to the caller
 *
 * This does not return until a process is ready to run
 */
Proc* nextRunnableProc(void) {
    Proc* p;
    Proc* save;

    syslock(&runQLock);
    enterCriticalRegion();

    LIST_FOR_EACH_ENTRY_SAFE(p, save, &procRunQ, nextRunQ) {
        if (p->state == ProcDead) {
            listUnlink(&p->nextRunQ);
            recycleProc(p);
        } else {
            processSignals(p);
            if (p->state == ProcReady) {
                listUnlinkAndInit(&p->nextRunQ);
                break;
            }
        }
    }

    if (p->state != ProcReady) {
        p = rp; /* nothing ready, cycle another quantum */
    } else if (rp != NULL) {
        listAddBefore(&rp->nextRunQ, &procRunQ);
    }

    leaveCriticalRegion();
    sysunlock(&runQLock);
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
        processSignals(rp);
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

int syssleep(long millis) {
    int fd = sysopen("/dev/timer/k70MilliTimer", CAP_WRITE);
    ptrdiff_t status = syswrite(fd, &millis, sizeof millis);
    sysclose(fd);
    return status;
}
