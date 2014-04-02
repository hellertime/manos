#include <errno.h>
#include <manos.h>
#include <string.h>

/*
 * General staticNS read function handles magic
 */
ptrdiff_t readStaticNS(Portal* p, const StaticNS* sns, void* buf, size_t size, Offset offset) {
    if (size == 0) return 0;

    unsigned i = STATICNS_CRUMB_SELF_IDX(p->crumb);
    const StaticNS* snsEnt = &sns[i];
    if (snsEnt->crumb.flags & CRUMB_ISDIR) {
        Portal px;
        NodeInfo ni;

        clonePortal(p, &px);
        NodeInfo* nix = getNodeInfoStaticNS(&px, sns, WalkDown, &ni);

        size_t bytes = size;
        char* c = buf;

        Offset skip = offset;
        Offset entries = 0;

        while (nix && bytes && (strlen(ni.name)+1) <= bytes) {
            /* for directories offset is treated as an integral dir index */
            px.crumb = ni.crumb;
            if (skip) {
                skip--;
            } else {
                memcpy(c, ni.name, strlen(ni.name));
                c   += strlen(ni.name);
                *c++ = 0;
                bytes -= strlen(ni.name) + 1;
                entries++;
            }
            nix = getNodeInfoStaticNS(&px, sns, WalkNext, &ni);
        }

        p->offset += entries;
        return entries;
    } else {
        if (snsEnt->length == 0) return 0;
        
        size_t bytes = size > snsEnt->length ? snsEnt->length : size;
        memcpy(buf, snsEnt->contents, snsEnt->length);
        p->offset += bytes;
        return bytes;
    }
}
