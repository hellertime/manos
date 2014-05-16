#include <manos.h>
#include <manos/list.h>
#include <arch/k70/derivative.h>
#include <string.h>

static Proc badProc = {
    .state           = ProcDead
,   .descriptorTable = {0}
,   .waitQ           = LIST_HEAD_INIT(badProc.waitQ)
,   .nextWaitQ       = LIST_HEAD_INIT(badProc.nextWaitQ)
,   .nextRunQ        = LIST_HEAD_INIT(badProc.nextRunQ)
,   .nextFreelist    = LIST_HEAD_INIT(badProc.nextFreelist)
,   .sigPending      = 0
,   .sigMask         = (uint32_t)-1
,   .pgrp            = 0
,   .ppid            = 0
,   .pid             = -1
,   .sp              = 0
};

static void processSignals(Proc* p) {
    static char buf[32];
    uint32_t newPending = 0; /* allow signals to generate signals */
    if (p->sigPending & SigAbort) {
        wakeWaiting(p);
        for (unsigned i = 0; i < COUNT_OF(p->descriptorTable); i++) {
            syskfree(p->descriptorTable[i]);
        }
        p->state = ProcDead;
        int len = fmtSnprintf(buf, sizeof buf, "\nKilled [%d]\n", p->pid);
        syswrite(rp->tty, buf, len);
    } else if (p->sigPending & SigStop) {
        int len = fmtSnprintf(buf, sizeof buf, "\nStopped [%d]\n", p->pid);
        syswrite(rp->tty, buf, len);
        wakeWaiting(p);
        INIT_LIST_HEAD(&p->waitQ);
        p->state = ProcStopped;
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
    int foundReady = 0;

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
                foundReady = 1;
                break;
            }
        }
    }

    leaveCriticalRegion();
    sysunlock(&runQLock);
    return (foundReady ? p : &badProc);
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
        listAddBefore(&rp->nextRunQ, &procRunQ);
        rp->sp = sp;
    } 

    rp = &badProc;
    while (rp == &badProc)
        rp = nextRunnableProc();

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
    if (status > 0) {
        enterCriticalRegion();
        rp->state = ProcWaiting;
        YIELD();
        leaveCriticalRegion();
    }
    return status;
}
