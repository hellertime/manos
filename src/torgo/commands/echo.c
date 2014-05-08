#include <manos.h>

/*
 * HACK ALERT. We don't have shell io redirection yet.
 * But 'echo' makes it look like we do!
 */
int cmdEcho__Main(int argc, char * const argv[]) {
    const char* out = NULL;
    int elems = argc;
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (*argv[i] == '>') {
                elems = i;

                if (*(argv[i] + 1)) {
                    out = (argv[i] + 1);
                    break;
                } else if ((i + 1) < argc) {
                    out = argv[i + 1];
                    break;
                }

                fprintln(rp->tty, "Bad Redirection");
                return -1;
            }
        }
    }

    int fd = rp->tty;
    if (out) {
        int fd2 = kopen(out, CAP_WRITE);
        if (fd2 < 0) {
            fprintln(rp->tty, "Cannot write to %s", out);
            return -1;
        }
        fd = fd2;
    }

    for (int i = 1; i < elems; i++) {
        if (i > 1) fputchar(fd, ' ');
        fputstr(fd, argv[i]);
    }
    fputchar(fd, '\n');

    if (out)
        kclose(fd);

    return 0;
}
