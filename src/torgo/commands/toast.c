#include <manos.h>

const char toast[] = "\n\t\t"
    " _   __\n\t\t"
    "( `^` ))\n\t\t"
    "|     ||\n\t\t"
    "|     ||\n\t\t"
    "'-----'`\n\n";

void showToast(void) {
    /* this has to cheat and use direct calls that bypass the syscall layer */
    sysprint(toast);
}

int cmdToast__Main(int argc, char * const argv[]) {
    UNUSED(argc);
    UNUSED(argv);

#ifdef PLATFORM_K70CW
    int fd = kopen("/dev/timer/k70PDB0", CAP_WRITE);
    kwrite(fd, &showToast, sizeof &showToast);
    kclose(fd);
    fprintln(u->tty, "Get ready for toast...");
#else
    showToast();
#endif
    return 0;
}
