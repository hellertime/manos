#include <assert.h>
#include <manos.h>
#include <manos/list.h>

extern void* __kmalloc(size_t, int);

Proc* newProc(void) {
    Proc* p;

    lock(&freelistLock);
    while (listIsEmpty(&procFreelist)) {
        unlock(&freelistLock);
        /* TODO: sleep() */
        lock(&freelistLock);
    }
    p = CONTAINER_OF(&procFreelist, Proc, nextFreelist);
    listUnlink(&procFreelist);
    unlock(&freelistLock);

    p->state = ProcSpawning;
    INIT_LIST_HEAD(&p->nextRunQ);
    INIT_LIST_HEAD(&p->nextFreelist);
    if (!p->pid) /* reuse existing pids -- only 127 available */
        p->pid = incRef(&nextPid);
    assert(p->pid != 0 && "newProc() pid has id 0");
    if (!p->stack)
        p->stack = __kmalloc(MANOS_ARCH_K70_STACK_SIZE, 0); /* kernel owns proc stack memory always -- since it is reused */

    return p;
}

