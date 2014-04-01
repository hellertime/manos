#include <manos.h>

void close(Portal* p) {
    deviceTable[p->device]->close(p);
}
