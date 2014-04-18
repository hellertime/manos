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
    uintptr_t action = (uintptr_t)&showToast;
    int fd = kopen("/dev/timer/k70OneShot", CAP_WRITE);
    ptrdiff_t status = kwrite(fd, &action, sizeof action);
    kclose(fd);

    if (status == -1)
        fprintln(u->tty, "Someone's making toast...");
    else
        fprintln(u->tty, "Get ready for toast...");
#else
    showToast();
#endif
    return 0;
}
