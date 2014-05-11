#include <manos.h>
#include <manos/list.h>

void recycleProc(Proc* p) {
    p->state = ProcDead;
    INIT_LIST_HEAD(&p->waitQ);
    INIT_LIST_HEAD(&p->nextWaitQ);
    INIT_LIST_HEAD(&p->nextRunQ);
    p->sp = 0;
    syslock(&freelistLock);
    listAddBefore(&p->nextFreelist, &procFreelist);
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
    if (!p->stack)
        p->stack = syskmalloc0(MANOS_ARCH_K70_STACK_SIZE); /* kernel owns proc stack memory always -- since it is reused */

    return p;
}

