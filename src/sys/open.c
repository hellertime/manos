#include <manos.h>

Portal* open(Portal* p, Caps caps) {
    return deviceTable[p->device]->open(p, caps);
}
