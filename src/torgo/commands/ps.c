#include <manos.h>
#include <manos/list.h>

void printProc(Proc* p) {
    const char* state;

    switch (p->state) {
    case ProcDead:
        state = "Dead";
        break;
    case ProcSpawning:
        state = "Spawn";
        break;
    case ProcReady:
        state = "Ready";
        break;
    case ProcRunning:
        state = "Run";
        break;
    case ProcWaiting:
        state = "Wait";
        break;
    default:
        state = "Unknown";
        break;
    }
    fprintln(rp->tty, "%d\t%d\t%s\t%s", p->pid, p->pgrp->pgid, state, p->argv[0]);
}

int cmdPs__Main(int argc, char * const argv[]) {
    Proc* p;
    fprintln(rp->tty, "PID\tPGID\tSTATE\tCMD");
    lock(&runQLock);
    printProc(rp);
    LIST_FOR_EACH_ENTRY(p, &procRunQ, nextRunQ) {
        printProc(p);
    }
    unlock(&runQLock);
    UNUSED(argc);
    UNUSED(argv);
    return 0;
}
