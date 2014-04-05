#include <manos.h>
#include <stdio.h>

int cmdCat__Main(int argc, char * const argv[]) {
    if (argc < 2) {
        sysprintln("usage: cat PATH");
        return -1;
    }

    NodeInfo ni;
    int fd = sysopen(argv[1], CAP_READ);
    sysgetInfoFd(fd, &ni);

    /* a very slow cat */
    char c;
    while (sysread(fd, &c, 1) == 1) {
        sysputchar(c);
    }

    sysclose(fd);
    return 0;
}
