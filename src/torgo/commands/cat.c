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

    int sw1 = kopen("/dev/swpb/1", CAP_READ);

    NodeInfo ni;
    int fd = kopen(path, CAP_READ);
    kfstat(fd, &ni);

    uint32_t x;
    char c;
    while (kread(fd, &x, inHex ? sizeof x : 1)) {
        if (inHex)
            fprint(u->tty, "0x%" PRIx32 "", x);
        else
            fputchar(u->tty, x);

        if (withNewlines) fputchar(u->tty, '\n');

        c = 0;
        kread(sw1, &c, 1);
        if (c == '0')
            break;
    }

    kclose(sw1);
    kclose(fd);
    return 0;
}
