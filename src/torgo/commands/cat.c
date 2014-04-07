#include <manos.h>

int cmdCat__Main(int argc, char * const argv[]) {
    if (argc < 2) {
        fprintln(u->tty, "usage: cat [OPTIONS] PATH");
        fprintln(u->tty, "\nOPTIONS:\n");
        fprintln(u->tty, "  -x                Read in 4 byte chunks and print as hex");
        fprintln(u->tty, "  -n                Emit newlines after each read");
        return -1;
    }

    char* path = NULL;
    int inHex = 0;
    int withNewlines = 0;

    for (int i = 1; i < argc; i++) {
        switch (*argv[i]) {
        case '-':
            if (*(argv[i]+1) == 'x') {
                inHex = 1;
            } else if (*(argv[i]+1) == 'n') {
                withNewlines = 1;
            }
            break;
        default:
            path = argv[i];
            i = argc;
            break;
        }
    }

    NodeInfo ni;
    int fd = kopen(path, CAP_READ);
    kfstat(fd, &ni);

    if (inHex) {
        uint32_t n;
        while (kread(fd, &n, sizeof n) == sizeof n) {
            fprint(u->tty, "%08x", n);
            if (withNewlines) fputchar(u->tty, '\n');
        }
    } else {
        char c;
        while (kread(fd, &c, 1) == 1) {
            fputchar(u->tty, c);
            if (withNewlines) fputchar(u->tty, '\n');
        }
    }

    kclose(fd);
    return 0;
}
