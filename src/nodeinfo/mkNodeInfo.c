#include <manos.h>

NodeInfo* mkNodeInfo(const Portal* p, Crumb c, const char *name, Offset length, Mode mode, NodeInfo* ni) {
    ni->name   = (char*)name;
    ni->crumb  = c;
    ni->device = p->device;
    ni->mode   = mode;
    ni->atime  = 0;
    ni->mtime  = 0;
    ni->length = length;
    return ni;
}
