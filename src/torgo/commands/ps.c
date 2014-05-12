#include <manos.h>

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
    fprintln("%d\t\t%s\t\t%s", p->pid, state, p->argv[0]);
}

int cmdPs__Main(int argc, char * const argv[]) {
    Proc* p;
    fprintln("PID\t\tSTATE\t\tCMD");
    lock(&runQLock);
    printProc(rp);
    FOR_EACH_ENTRY(p, procRunQ, nextRunQ) {
        printProc(p);
    }
    unlock(&runQLock);
    return 0;
}