#include <errno.h>
#include <manos.h>

/* 
 * Note that a StaticNS node entry will have a Crumb
 * with an fid that is a compound value containing
 * the parent index in the high bytes and the child index
 * in the low bytes. Since a Fid is 32-bits this gives
 * us a max of 2^16 possible entries in a static namespace.
 * The value 2^16 is reserved leaving 2^16-1 values which 
 * is a reasonable amount given that these StaticNS objects
 * are usually constructed by hand -- note that the index
 * values must be assigned in breadth first order on a tree where
 * node siblings are the left branches and children are the right
 * The root should always be '0'
 *
 * The last entry in the static namespace should have both its self
 * and parent index values be 2^16, this acts as a sentinel value
 */
NodeInfo* getNodeInfoStaticNS(const Portal* p, const StaticNS* ns, WalkDirection d, NodeInfo *ni) {
    ASSERT(PORTAL_ISSTATICNS(p) && "Portal is not focuse on a StaticNS path");

    const StaticNS* sns = NULL;

    if (!ns) {
        errno = EINVAL;
        return NULL;
    }

    StaticIndex parentIdx = STATICNS_CRUMB_PARENT_IDX(p->crumb);
    StaticIndex selfIdx   = STATICNS_CRUMB_SELF_IDX(p->crumb);

    switch (d) {
    case WalkUp:
        if (selfIdx != 0) { 
            sns = &ns[parentIdx];
        } else {
            sns = &ns[selfIdx];  /* moving up from /.. always yields / */
        }
        break;
    case WalkDown:
       if(!PORTAL_ISDIR(p)) {
           errno = ENOTDIR;
           return NULL;
       }
       for (unsigned i = selfIdx; ; i++) {
           if (STATICNS_CRUMB_PARENT_IDX(ns[i].crumb) == selfIdx) {
               sns = &ns[i];
               break;
           }

           if (STATICNS_CRUMB_ISSENTINEL(ns[i].crumb))
               goto notfound;
       }
       break;
    case WalkPrev:
        sns = &ns[selfIdx - 1];
        if (parentIdx != STATICNS_CRUMB_PARENT_IDX(sns->crumb))
            goto notfound;
        break;
    case WalkNext:
        sns = &ns[selfIdx + 1];
        if (parentIdx != STATICNS_CRUMB_PARENT_IDX(sns->crumb))
            goto notfound;
        break;
    case WalkSelf:
        sns = &ns[selfIdx];
        break;
    }

    ni->contents = sns->contents;
    return mkNodeInfo(p, sns->crumb, sns->name, sns->length, sns->mode, ni);

notfound:
    errno = ENOENT;
    return NULL;
}
