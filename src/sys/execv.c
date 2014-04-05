#include <errno.h>
#include <manos.h>
#include <torgo/commands.h>
#include <string.h>

/*
 * This is a hack syscall which 'executes' a file.
 * These files must be mode 0555 and have their contents
 * begin with #!<x> where <x> will map to a builtin
 *
 * TOTAL HACK!
 */
int sysexecv(const char *path, char * const argv[]) {
    int argc;
    int ret = -1;
    char *buf = NULL;
    Portal* p = NULL;

    if (!path) {
        errno = EINVAL;
        goto error;
    }

    int isRel = *path != '/';
    /*
     * While various standards exist for maximum args (ARG_MAX, _SC_ARG_MAX)
     * Since our runtime is stll being defined for now we just punt at 256
     */
    for (argc = 0; argc < 256; argc++) {
        if (argv[argc] == NULL) /* this function mandates that argv[argc] == NULL */
            break;
    }

    errno = EPERM;

    Path* pth = mkPath(path);
    if (!pth)
        goto error;

    /* Simple command searching. If the command isn't found on the first walk:
     * If the path is relative, try again from /bin
     */
    if (((p = syswalk(isRel?u->dot:u->slash, pth->elems, pth->nelems)) == NULL) && isRel) {
        char *bin = "bin";
        Portal* pbin = syswalk(u->slash, &bin, 1);
        if (!pbin) {
            errno = EIO;
            goto error;
        }
        p = syswalk(pbin, pth->elems + pth->nelems - 1, 1);
        kfree(pbin);
    }
    if (p && (p->crumb.flags & CRUMB_ISFILE)) {
        NodeInfo ni;
        if (deviceTable[p->device]->getInfo(p, &ni) == -1)
            goto error;

        if (ni.mode != 0555) {
            errno = EPERM;
            goto error;
        }

        if (deviceTable[p->device]->open(p, CAP_READ) == NULL)
            goto error;

        buf = kmallocz(ni.length + 1);
        if (deviceTable[p->device]->read(p, buf, ni.length, 0) == -1)
            return -1;

        deviceTable[p->device]->close(p);

        char* c = buf;
        if (c[0] != '#' || c[1] != '!') {
            errno = EPERM;
            goto error;
        }

        c += 2;
        for (unsigned i = 0; i < COUNT_OF(builtinCmds); i++) {
            if (strcmp(builtinCmds[i].cmdName, c) == 0) {
                ret = builtinCmds[i].cmd(argc, argv);
                break;
            }
        }
    }

error:
    if (pth) kfree(pth);
    if (p)   kfree(p);
    if (buf) kfree(buf);
    return ret;
}
