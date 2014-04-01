#include <manos.h>
#include <stdio.h>

int cmdPwd__Main(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
    char buf[4096];
    getCwd(buf, sizeof buf);
    puts(buf);
    putchar('\n');
    return 0;
}
