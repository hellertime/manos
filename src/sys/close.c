#include <manos.h>

void sysclose(int fd) {
    Portal* p = rp->descriptorTable[fd];
    if (p) {
        if (p->device >= MANOS_MAXDEV) {
            sysprintln("sysclose() unknown device %d", p->device);
            return;
        }
        deviceTable[p->device]->close(p);
        syskfree(p);
        rp->descriptorTable[fd] = 0;
    }
}
