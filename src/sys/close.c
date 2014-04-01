#include <manos.h>

void sysclose(int fd) {
    Portal* p = descriptorTable[fd];
    if (p) {
        deviceTable[p->device]->close(p);
        kfree(p);
        descriptorTable[fd] = 0;
    }
}
