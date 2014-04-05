#include <errno.h>
#include <manos.h>

int sysopen(const char* path, Caps caps) {
    Portal* p = NULL;
    int isRel = *path != '/';

    errno = EPERM;

    Path* pth = mkPath(path);
    if (!pth)
        goto error;

    if((p = syswalk(isRel?u->dot:u->slash, pth->elems, pth->nelems)) == NULL)
        goto error;

    int fd = -1;
    for (fd = 0; fd < MANOS_MAXFD; fd++) {
        if (u->descriptorTable[fd] == NULL) {
            u->descriptorTable[fd] = deviceTable[p->device]->open(p, caps);
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
