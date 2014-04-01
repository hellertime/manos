#include <assert.h>
#include <errno.h>
#include <manos.h>
#include <string.h>

static StaticNS rootSNS[] = {
    /* root */
    { "/", MKSTATICNS_CRUMB(STATICNS_SENTINEL, 0, CRUMB_ISDIR), 0, 0555 }

    /* first level */
,   { "bin", MKSTATICNS_CRUMB(0, 1, CRUMB_ISDIR), 0, 0555 }
,   { "dev", MKSTATICNS_CRUMB(0, 2, CRUMB_ISDIR), 0, 0555 }

    /* children of bin */
,   { "pwd", MKSTATICNS_CRUMB(1, 3, CRUMB_ISFILE), 3, 0555 }

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
    return getNodeInfoStaticNS(p, rootSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

static ptrdiff_t readRoot(Portal* p, void* buf, size_t size, Offset offset) {
    UNUSED(offset);
    if (size == 0) return 0;

    unsigned i = STATICNS_CRUMB_SELF_IDX(p->crumb);
    assert(i < COUNT_OF(rootSNS) - 1);
    StaticNS* sns = &rootSNS[i];
    if (sns->length == 0) return 0;
    size_t bytes = size > sns->length ? sns->length : size;
    memcpy(buf, sns->name, bytes);
    return bytes;
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
