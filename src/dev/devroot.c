#include <assert.h>
#include <errno.h>
#include <manos.h>
#include <string.h>

static struct Contents {
    char *name;
    size_t size;
    char *cnt;
} contents[] = {
    { "pwd", 5, "#!pwd" }
,   { "ls", 4, "#!ls" }
};

static StaticNS rootSNS[] = {
    /* root */
    { "/", MKSTATICNS_CRUMB(STATICNS_SENTINEL, 0, CRUMB_ISDIR), 0, 0555 }

    /* first level */
,   { "bin", MKSTATICNS_CRUMB(0, 1, CRUMB_ISDIR), 0, 0555 }
,   { "dev", MKSTATICNS_CRUMB(0, 2, CRUMB_ISDIR), 0, 0555 }

    /* children of bin */
,   { "pwd", MKSTATICNS_CRUMB(1, 3, CRUMB_ISFILE), 5, 0555 }
,   { "ls", MKSTATICNS_CRUMB(1, 4, CRUMB_ISFILE), 4, 0555 }

    /* sentinel */
,   { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0 }
};

static Portal* attachRoot(char* path) {
    Portal* p = attachDev(DEV_DEVROOT, path);
    p->crumb = rootSNS[0].crumb;
    return p;
}

static NodeInfo* rootNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, rootSNS, d, ni);
}

static WalkTrail* walkRoot(Portal* p, char** path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, rootNodeInfoFn);
}

static Portal* openRoot(Portal* p, Caps caps) {
    return openDev(p, caps);
}

static void closeRoot(Portal* p) {
    UNUSED(p);
    errno = EPERM;
}

static int getInfoRoot(const Portal *p, NodeInfo* ni) {
    if (getNodeInfoStaticNS(p, rootSNS, WalkSelf, ni) == NULL)
        return -1;

    if (ni->crumb.flags & CRUMB_ISDIR) {
        Portal   px;
        NodeInfo nip;

        clonePortal(p,&px);
        NodeInfo* nix = getNodeInfoStaticNS(&px, rootSNS, WalkDown, &nip);

        size_t length = 0;
        while (nix) {
            px.crumb = nip.crumb;
            length += strlen(nip.name) + 1; /* name + '\n' */
            nix = getNodeInfoStaticNS(&px, rootSNS, WalkNext, &nip);
        }

        if (length) length++; /* +1 for \0 */
        ni->length = length;
    }

    return 0;
}

static ptrdiff_t readRoot(Portal* p, void* buf, size_t size, Offset offset) {
    if (size == 0) return 0;

    unsigned i = STATICNS_CRUMB_SELF_IDX(p->crumb);
    assert(i < COUNT_OF(rootSNS) - 1);
    StaticNS* sns = &rootSNS[i];
    if (sns->crumb.flags & CRUMB_ISDIR) {
        Portal px;
        NodeInfo ni;

        clonePortal(p, &px);
        NodeInfo* nix = getNodeInfoStaticNS(&px, rootSNS, WalkDown, &ni);

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
            nix = getNodeInfoStaticNS(&px, rootSNS, WalkNext, &ni);
        }

        return entries;
    } else {
        if (sns->length == 0) return 0;

        for (unsigned i = 0; i < COUNT_OF(contents); i++) {
            if (strcmp(contents[i].name, sns->name) == 0) {
                size_t bytes = size > sns->length ? sns->length : size;
                memcpy(buf, contents[i].cnt, sns->length);
                return bytes;
            }
        }
        return 0;
    }
}

static ptrdiff_t writeRoot(Portal* p, void* buf, size_t size, Offset offset) {
    UNUSED(p);
    UNUSED(buf);
    UNUSED(size);
    UNUSED(offset);
    errno = EPERM;
    return -1;
}

Dev devRoot = {
    .id       = DEV_DEVROOT
,   .name     = "root"
,   .init     = initDev
,   .reset    = resetDev
,   .shutdown = shutdownDev
,   .attach   = attachRoot
,   .walk     = walkRoot
,   .create   = createDev
,   .open     = openRoot
,   .close    = closeRoot
,   .remove   = removeDev
,   .getInfo  = getInfoRoot
,   .setInfo  = setInfoDev
,   .read     = readRoot
,   .write    = writeRoot
};
