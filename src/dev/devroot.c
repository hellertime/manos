#include <errno.h>
#include <manos.h>
#include <string.h>

/*
 * Devroot - Magic namespace.
 *
 * Devroot is a hack. Since namespace mounting is not yet 
 * implemented, instead the static namespace is specialized
 * into a root 'device' which binds together the other device
 * namespaces.
 *
 * A couple of fields are overloaded to give us some features
 * that aren't fully realized yet.
 *
 * The first is the 'executable' format. A file with mode 0555,
 * and with special contents will be executable by sysexec.
 * This is sure to change shortly.
 *
 * The second hack is mount points. If a node has the CRUMB_ISMOUNT
 * flag set, its length field is reused to hold the mount point
 * device id. Ugly.
 */

#define NAMESPACE_MAP     \
    X(".", STATICNS_SENTINEL, Dot, CRUMB_ISDIR, 0, 0555, 0) \
    X("bin", FidDot, Bin, CRUMB_ISDIR, 0, 0555, 0) \
    X("dev", FidDot, Dev, CRUMB_ISDIR, 0, 0555, 0) \
    X("pwd", FidBin, BinPwd, CRUMB_ISFILE, 5, 0555, "#!pwd") \
    X("ls", FidBin, BinLs, CRUMB_ISFILE, 4, 0555, "#!ls") \
    X("cat", FidBin, BinCat, CRUMB_ISFILE, 5, 0555, "#!cat") \
    X("echo", FidBin, BinEcho, CRUMB_ISFILE, 6, 0555, "#!echo") \
    X("swpb", FidDev, DevSwpb, CRUMB_ISMOUNT, DEV_DEVSWPB, 0444, 0) \
    X("led", FidDev, DevLed, CRUMB_ISMOUNT, DEV_DEVLED, 0444, 0) \
    X("uart", FidDev, DevUart, CRUMB_ISMOUNT, DEV_DEVUART, 0444, 0) \
    X("lcd", FidDev, DevLcd, CRUMB_ISMOUNT, DEV_DEVLCD, 0444, 0) \
    X("adc", FidDev, DevAdc, CRUMB_ISMOUNT, DEV_DEVADC, 0444, 0)

#define X(p, u, s, t, z, m, c) Fid##s,
typedef enum {
NAMESPACE_MAP
} RootFidEnt;
#undef X

#define X(p, u, s, t, z, m, c) { p, MKSTATICNS_CRUMB(u, Fid##s, t), z, m, c },
static StaticNS rootSNS[] = {
NAMESPACE_MAP
    { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0, 0 }
};
#undef X

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
    return readStaticNS(p, rootSNS, buf, size, offset);
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
,   .power    = powerDev
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
