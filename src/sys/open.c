#include <errno.h>
#include <manos.h>

int __sysopen(Proc* rp, const char* path, Caps caps) {
    Portal* p = NULL;
    int isRel = *path != '/';

    errno = EPERM;

    Path* pth = mkPath(path);
    if (!pth)
        goto error;

    if((p = syswalk(isRel?rp->dot:rp->slash, pth->elems, pth->nelems)) == NULL)
        goto error;

    int fd = -1;
    for (fd = 0; fd < MANOS_MAXFD; fd++) {
        if (rp->descriptorTable[fd] == NULL) {
            rp->descriptorTable[fd] = deviceTable[p->device]->open(p, caps);
            return fd;
        }
    }

    if (fd == MANOS_MAXFD) {
        errno = EMFILE;
    }

error:
    if (pth) syskfree(pth);
    if (p) syskfree(p);
    return -1;
}

int sysopen(const char* path, Caps caps) {
    return __sysopen(rp, path, caps);
}
