#include <manos.h>

const char toast[] = "\n\t\t\t\t"
    " _   __\n\t\t\t\t"
    "( `^` ))\n\t\t\t\t"
    "|     ||\n\t\t\t\t"
    "|     ||\n\t\t\t\t"
    "'-----'`\n\n";

void showToast(void) {
    /* this has to cheat and use direct calls that bypass the syscall layer */
    sysprint(toast);
}

int cmdToast__Main(int argc, char * const argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    fprintln(rp->tty, "Get ready for toast...");
    sleep(2 * 1000); /* 2 second sleep */
    showToast();
    return 0;
}
