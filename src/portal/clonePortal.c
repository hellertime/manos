#include <manos.h>

Portal* clonePortal(const Portal* p, Portal* px) {
    px->device = p->device;
    px->crumb  = p->crumb;
    return px;
}
