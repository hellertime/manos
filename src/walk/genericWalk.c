WalkTrail* genericWalk(const Portal* p, const char** path, unsigned n, GetNodeInfoFn *fn) {
    NodeInfo ni = {0};
    Portal   px = {0};

    if (!n) {
        errno = EINVAL;
        return NULL;
    }  

    if (!PORTAL_ISDIR(p)) {
       errno = ENOTDIR;
       return NULL;
    }

    errno = 0;
    if((px = clonePortal(p, &px)) == NULL) {
        errno = errno ? errno : ENOTRECOVERABLE;
        return NULL;
    }

    WalkTrail* t = emptyWalkTrail(n);
    if (!t) {
        closePortal(&px);
        return NULL;
    }

    for (int i = 0; i < n; i++) {
        if (!PORTAL_ISDIR(&px))
            break;

        const char *name = path[i];

        if (streq(name, ".")) {
            fn(&px, WalkSelf, &ni);
            px.crumb = ni.crumb;
            pushCrumb(t, &px->crumb);
            continue;
        } else if (streq(name, "..")) {
           fn(&px, WalkUp, &ni);
           px.crumb = ni.crumb;
           pushCrumb(t, &px->crumb);
           continue;
        }

        NodeInfo* nix = fn(&px, WalkDown, &ni);

        while (nix) {
            if (streq(name, ni.name)) {
                px.crumb = ni.crumb;
                pushCrumb(t, &px->crumb);
                break;
            }

            nix = fn(&px, WalkNext, &ni);
        }

        if (!nix)
            break;
    }

    closePortal(&px);
    return t;
}
