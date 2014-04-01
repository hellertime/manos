#include <manos.h>
#include <stdio.h>

int cmdPwd__Main(int argc, char * const argv[]) {
    UNUSED(argc);
    UNUSED(argv);
    char buf[4096];
    getcwd(buf, sizeof buf);
    puts(buf);
    putchar('\n');
    return 0;
}
