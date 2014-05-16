#include <manos.h>
#include <manos/list.h>
#include <string.h>

uint64_t canary = 0xdecade0fc0ffecab;

ProcGroup* newProcGroup(int pgid) {
    ProcGroup* pgrp = syskmalloc0(sizeof *pgrp);
    INIT_REF(&pgrp->memberCount);
    incRef(&pgrp->memberCount);
    pgrp->pgid = pgid;
    return pgrp;
}

void leaveProcGroup(ProcGroup* pgrp) {
    if (decRef(&pgrp->memberCount) == 0) {
        syskfree(pgrp);
    }
}

void joinProcGroup(ProcGroup* pgrp, Proc* p) {
    leaveProcGroup(p->pgrp);
    incRef(&pgrp->memberCount);
    p->pgrp = pgrp;
}

void abortProc(Proc* p) {
    wakeWaiting(p);
    listUnlinkAndInit(&p->nextWaitQ);
    for (unsigned i = 0; i < COUNT_OF(p->descriptorTable); i++) {
        kfree(p->descriptorTable[i]);
    }
}

void recycleProc(Proc* p) {
    p->state = ProcDead;
    kmemset(p->descriptorTable, 0, sizeof (p->descriptorTable));
    INIT_LIST_HEAD(&p->waitQ);
    INIT_LIST_HEAD(&p->nextWaitQ);
    INIT_LIST_HEAD(&p->nextRunQ);
    INIT_LIST_HEAD(&p->nextFreelist);
    p->sigPending = 0;
    p->sigMask    = 0;
    leaveProcGroup(p->pgrp);
    p->pgrp = 0;
    p->ppid = 0;
    p->sp = 0;
    syslock(&freelistLock);
    listAddBefore(&p->nextFreelist, &procFreelist);
    procTable[p->pid] = 0;
    sysunlock(&freelistLock);
}

Proc* newProc(void) {
    Proc* p;

    syslock(&freelistLock);
    while (listIsEmpty(&procFreelist)) {
        sysunlock(&freelistLock);
        /* TODO: sleep() */
        syslock(&freelistLock);
    }
    p = CONTAINER_OF((&procFreelist)->next, Proc, nextFreelist);
    listUnlink(&p->nextFreelist);
    sysunlock(&freelistLock);

    p->state = ProcSpawning;
    INIT_LIST_HEAD(&p->waitQ);
    INIT_LIST_HEAD(&p->nextWaitQ);
    INIT_LIST_HEAD(&p->nextRunQ);
    INIT_LIST_HEAD(&p->nextFreelist);
    if (!p->pid) /* reuse existing pids -- only 127 available */
        p->pid = incRef(&nextPid);
    ASSERT(p->pid != 0 && "newProc() pid has id 0");
    if (!p->stack) {
        char* stack = syskmalloc0(MANOS_ARCH_K70_STACK_SIZE + (4 * sizeof(uint32_t))); /* kernel owns proc stack memory always -- since it is reused */
        p->canary1 = (uint64_t*)stack;
        memcpy(p->canary1, &canary, sizeof canary);
        stack = stack + sizeof(canary);
        p->canary2 = (uint64_t*)(stack + MANOS_ARCH_K70_STACK_SIZE);
        memcpy(p->canary2, &canary, sizeof(canary));
        p->stack = (uint32_t*)stack;
        ASSERT(*p->canary1 == *p->canary2 && "newProc() canaries are not equal");
    }
    p->pgrp = newProcGroup(p->pid);
    p->ppid = rp ? rp->pid : 0;
    p->sigPending = 0;
    p->sigMask    = 0;
    ASSERT(procTable[p->pid] == NULL && "newProc() existing proc in table");
    procTable[p->pid] = p;

    return p;
}

