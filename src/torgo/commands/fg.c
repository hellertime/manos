#include <manos.h>

int cmdFg__Main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(rp->tty, "usage: %s PID\n", argv[0]);
        return 1;
    }

    syspostsignal(atoi(argv[1]), SigContinue);
    return 0;
}
