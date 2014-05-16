#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>
#include <string.h>

static void processSignals(Proc* p) {
    static char buf[32];
    uint32_t newPending = 0; /* allow signals to generate signals */
    if (p->sigPending & SigAbort) {
        abortProc(p);
        p->state = ProcDead;
        int len = fmtSnprintf(buf, sizeof buf, "Killed [%d]\n", p->pid);
        syswrite(rp->tty, buf, len);
    } else if (p->sigPending & SigStop) {
        wakeWaiting(p);
        INIT_LIST_HEAD(&p->waitQ);
        p->state = ProcStopped;
        int len = fmtSnprintf(buf, sizeof buf, "Stopped [%d]", p->pid);
        syswrite(rp->tty, buf, len);
    } else if (p->sigPending & SigContinue) {
        p->state = ProcReady;
        listAddAfter(&procTable[p->ppid]->nextWaitQ, &p->waitQ);
        procTable[p->ppid]->state = ProcWaiting;
    } else if (p->sigPending & SigAlarm) {
        p->state = ProcReady;
    }
    p->sigPending = newPending;
}
/**
 * nextRunnableProc() - return a proc to run to the caller
 *
 * This does not return until a process is ready to run
 */
Proc* nextRunnableProc(void) {
    Proc* p = NULL;
    Proc* save = NULL;

    syslock(&runQLock);
    enterCriticalRegion();

    volatile int isEmpty = listIsEmpty(&procRunQ);
    UNUSED(isEmpty);
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

    if (p == NULL && rp->state == ProcReady) {
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

    Proc* p = NULL;
    while ((p = nextRunnableProc()) == NULL)
        ;

    rp = p;
    ASSERT(*rp->canary1 == *rp->canary2 && "scheduleProc() new proc canaries are not equal");
    ASSERT(rp->sp < (uintptr_t)rp->canary2 && "scheduleProc() new proc sp below canary");
    rp->state = ProcRunning;
    RESET_SYSTICK();
    START_SYSTICK();
    return rp->sp;
}

int syssleep(long millis) {
    char duration[21] = {0};
    fmtSnprintf(duration, sizeof duration, "%ld", millis);
    int fd = sysopen("/dev/timer/k70MilliTimer", CAP_WRITE);
    ptrdiff_t status = syswrite(fd, duration, strlen(duration));
    sysclose(fd);
    if (status == 0) {
        enterCriticalRegion();
        rp->state = ProcWaiting;
        YIELD();
        leaveCriticalRegion();
    }
    return status;
}
