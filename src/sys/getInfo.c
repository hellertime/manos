#include <errno.h>
#include <manos.h>

int sysgetInfoFd(int fd, NodeInfo* ni) {
    Portal* p = u->descriptorTable[fd];
    if (!p) {
        errno = EBADF;
        return -1;
    }

    if (p->crumb.flags & CRUMB_ISDIR) {
        errno = EISDIR;
        return -1;
    }

    return deviceTable[p->device]->getInfo(p, ni);
}

int setInfo(Portal* p, NodeInfo* ni) {
    return deviceTable[p->device]->setInfo(p, ni);
}
