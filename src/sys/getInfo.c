#include <errno.h>
#include <manos.h>

int getInfo(const Portal* p, NodeInfo* ni) {
    return deviceTable[p->device]->getInfo(p, ni);
}

int setInfo(Portal* p, NodeInfo* ni) {
    return deviceTable[p->device]->setInfo(p, ni);
}
