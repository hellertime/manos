#include <errno.h>
#include <manos.h>

int sysopen(const char* path, Caps caps) {
    int isRel = *path != '/';

    errno = EPERM;

    Path* pth = mkPath(path);
    if (!pth)
        goto error;

    Portal* p = syswalk(isRel ? dot : slash, pth->elems, pth->nelems);
    if (!p)
        goto error;

    int fd = -1;
    for (fd = 0; fd < MANOS_MAXFD; fd++) {
        if (descriptorTable[fd] == NULL) {
            descriptorTable[fd] = deviceTable[p->device]->open(p, caps);
            return fd;
        }
    }

    if (fd == MANOS_MAXFD) {
        errno = EMFILE;
    }

error:
    if (pth) kfree(pth);
    if (p) kfree(p);
    return -1;
}
