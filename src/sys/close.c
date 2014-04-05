#include <manos.h>

void sysclose(int fd) {
    Portal* p = u->descriptorTable[fd];
    if (p) {
        deviceTable[p->device]->close(p);
        kfree(p);
        u->descriptorTable[fd] = 0;
    }
}
