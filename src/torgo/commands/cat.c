#include <manos.h>

int cmdCat__Main(int argc, char * const argv[]) {
    if (argc < 2) {
        sysprintln("usage: cat PATH");
        return -1;
    }

    NodeInfo ni;
    int fd = open(argv[1], CAP_READ);
    fstat(fd, &ni);

    /* a very slow cat */
    char c;
    while (read(fd, &c, 1) == 1) {
        sysputchar(c);
    }

    close(fd);
    return 0;
}
