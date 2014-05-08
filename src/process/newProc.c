#include <assert.h>
#include <manos.h>
#include <manos/list.h>

Proc* newProc(void) {
    Proc* p;

    lock(&freelistLock);
    while (listIsEmpty(&procFreelist)) {
        unlock(&freelistLock);
        /* TODO: sleep() */
        lock(&freelistLock);
    }
    p = CONTAINER_OF(&procFreelist->next, Proc, nextFreelist);
    listUnlinkAndInit(&procFreelist->next);
    unlock(&freelistLock);

    p->state = ProcSpawning;
    INIT_LIST_HEAD(&p->nextWaitQ);
    INIT_LIST_HEAD(&p->nextRunQ);
    INIT_LIST_HEAD(&p->nextFreelist);
    if (!p->pid) /* reuse existing pids -- only 127 available */
        p->pid = incRef(&nextPid);
    assert(p->pid != 0 && "newProc() pid has id 0");
    if (!p->stack)
        p->stack = syskmalloc0(MANOS_ARCH_K70_STACK_SIZE); /* kernel owns proc stack memory always -- since it is reused */

    return p;
}

