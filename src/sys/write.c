#include <errno.h>
#include <manos.h>
#include <stddef.h>

ptrdiff_t syswrite(int fd, void* buf, size_t n) {
    Portal* p = rp->descriptorTable[fd];
    if (fd < 0 || !p) {
        errno = EBADF;
        return -1;
    }

    return deviceTable[p->device]->write(p, buf, n, p->offset);
}
