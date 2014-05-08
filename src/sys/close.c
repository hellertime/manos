#include <manos.h>

void sysclose(int fd) {
    Portal* p = rp->descriptorTable[fd];
    if (p) {
        deviceTable[p->device]->close(p);
        kfree(p);
        rp->descriptorTable[fd] = 0;
    }
}
