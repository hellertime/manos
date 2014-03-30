#include <errno.h>
#include <manos.h>
#include <stddef.h>
#include <string.h>

WalkTrail* genericWalk(const Portal* p, const char** path, unsigned n, GetNodeInfoFn fn) {
    NodeInfo ni;
    Portal   px;

    if (!n) {
        errno = EINVAL;
        return NULL;
    }  

    if (!PORTAL_ISDIR(p)) {
       errno = ENOTDIR;
       return NULL;
    }

    errno = 0;
    if(clonePortal(p, &px) == NULL) {
        errno = errno ? errno : ENOTRECOVERABLE;
        return NULL;
    }

    WalkTrail* t = emptyWalkTrail(n);
    if (!t) {
        closePortal(&px);
        return NULL;
    }

    for (unsigned i = 0; i < n; i++) {
        if (!PORTAL_ISDIR(&px)) {
            errno = ENOTDIR;
            break;
        }

        const char *name = path[i];

        if (strcmp(name, ".") == 0) {
            fn(&px, WalkSelf, &ni);
            px.crumb = ni.crumb;
            pushCrumb(t, px.crumb);
            continue;
        } else if (strcmp(name, "..") == 0) {
           fn(&px, WalkUp, &ni);
           px.crumb = ni.crumb;
           pushCrumb(t, px.crumb);
           continue;
        }

        NodeInfo* nix = fn(&px, WalkDown, &ni);

        while (nix) {
            px.crumb = ni.crumb;

            if (strcmp(name, ni.name) == 0) {
                pushCrumb(t, px.crumb);
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
