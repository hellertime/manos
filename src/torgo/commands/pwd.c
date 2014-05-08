#include <manos.h>

int cmdPwd__Main(int argc, char * const argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    char buf[4096];
    getcwd(buf, sizeof buf);
    fprintln(rp->tty, buf);
    return 0;
}
