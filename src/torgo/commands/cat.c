#include <manos.h>

int cmdCat__Main(int argc, char * const argv[]) {
    if (argc < 2) {
        sysprintln("usage: cat PATH");
        return -1;
    }

    NodeInfo ni;
    int fd = kopen(argv[1], CAP_READ);
    kfstat(fd, &ni);

    /* a very slow cat */
    char c;
    while (kread(fd, &c, 1) == 1) {
        sysputchar(c);
    }

    kclose(fd);
    return 0;
}
