#include <manos.h>
#include <stddef.h>

ptrdiff_t read(Portal* p, void* buf, size_t n) {
    return deviceTable[p->device]->read(p, buf, n, p->offset);
}
