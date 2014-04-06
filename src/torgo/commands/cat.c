#include <manos.h>

int cmdCat__Main(int argc, char * const argv[]) {
    if (argc < 2) {
        fprintln(u->tty, "usage: cat PATH");
        return -1;
    }

    NodeInfo ni;
    int fd = kopen(argv[1], CAP_READ);
    kfstat(fd, &ni);

    /* a very slow cat */
    char c;
    while (kread(fd, &c, 1) == 1) {
        fputchar(u->tty, c);
    }

    kclose(fd);
    return 0;
}
