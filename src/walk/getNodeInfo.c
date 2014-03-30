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
 */
NodeInfo* getNodeInfo(Portal* p, StaticNS* ns, Walk w, NodeInfo *ni) {
    ASSERT_NOT_BADPTR(ni);
    ASSERT_STATICNS(p);

    StaticNS* sns = NULL;

    if (!ns) {
        errno = EINVAL;
        return &BADPTR;
    }

    uint16_t parentIdx = GET_PARENT_IDX(p->crumb);
    uint16_t selfIdx   = GET_SELF_IDX(p->crumb);

    switch (w) {
    case WalkUp:
        sns = ns[parentIdx];
        break;
    case WalkPrev:
        sns = ns[selfIdx - 1];
        if (parentIdx != GET_PARENT_IDX(sns->crumb))
            goto notfound;
        break;
    case WalkNext:
        sns = ns[selfIdx + 1];
        if (parentIdx != GET_PARENT_IDX(sns->crumb))
            goto notfound;
        break;
    case WalkSelf:
        sns = ns[selfIdx];
        break;
    }

    return mkNodeInfo(p, sns->crumb, sns->name, sns->length, sns->perm, ni);

notfound:
    errno = ENOENT;
    return &BADPTR;
}
