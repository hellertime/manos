#include <errno.h>
#include <manos.h>
#include <stddef.h>

ptrdiff_t sysread(int fd, void* buf, size_t n) {
    Portal* p = u->descriptorTable[fd];
    if (!p) {
        errno = EBADF;
        return -1;
    }

    return deviceTable[p->device]->read(p, buf, n, p->offset);
}
