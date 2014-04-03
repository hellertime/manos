#include <manos.h>

Portal* mkPortal(Portal*p, DeviceIndex device) {
    p->device      = device;
    p->flags       = 0;
    p->crumb.fid   = 0;
    p->crumb.flags = 0;
    p->caps        = 0;
    p->offset      = 0;
    return p;
}
