NodeInfo* mkNodeInfo(Portal* p, Crumb c, const char *name, off_t length, mode_t mode, NodeInfo* ni) {
    ni->name   = name;
    ni->crumb  = c;
    ni->device = p->device;
    ni->mode   = mode | (c.type << 24);
    ni->atime  = 0;
    ni->mtime  = 0;
    ni->length = length;
    return ni;
}
