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
    int isRel = *path != '/';
    int ret = -1;
    char *buf = NULL;

    errno = EPERM;

    Path* pth = mkPath(path);
    if (!pth)
        goto error;

    Portal* p = syswalk(isRel ? dot : slash, pth->elems, pth->nelems);
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
                ret = builtinCmds[i].cmd(COUNT_OF(argv), argv);
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
