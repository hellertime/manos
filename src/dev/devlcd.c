#include <errno.h>
#include <manos.h>
#include <stdlib.h>

static Lcd* enableLcd(Lcd* lcd) {
    lcd->hw->enable(lcd);
    return lcd;
}

static void disableLcd(Lcd* lcd) {
    lcd->hw->disable(lcd);
}

#define NAMESPACE_MAP     \
    X(".", STATICNS_SENTINEL, Dot, CRUMB_ISDIR, 0, 0555, 0) \
    X("clear", FidDot, Clear, CRUMB_ISFILE, 0, 0644, 0) \
    X("fg", FidDot, Fg, CRUMB_ISFILE, 0, 0644, 0) \
    X("bg", FidDot, Bg, CRUMB_ISFILE, 0, 0644, 0) \
    X("blit", FidDot, Blit, CRUMB_ISFILE, 522240, 0222, 0)

#define X(p, u, s, t, z, m, c) Fid##s,
typedef enum {
NAMESPACE_MAP
} LcdFidEnt;
#undef X

#define X(p, u, s, t, z, m, c) { p, MKSTATICNS_CRUMB(u, Fid##s, t), z, m, c },
static StaticNS lcdSNS[] = {
NAMESPACE_MAP
    { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0, 0 }
};
#undef X

static Lcd* lcdScreen = NULL;

static void resetLcd(void) {
    if (lcdHw && lcdHw->hotplug)
        lcdScreen = lcdHw->hotplug();

    if (lcdScreen)
        enableLcd(lcdScreen);
}

static Portal* attachLcd(char* path) {
    Portal* p = attachDev(DEV_DEVLCD, path);
    p->crumb = lcdSNS[0].crumb;
    return p;
}

static NodeInfo* lcdNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, lcdSNS, d, ni);
}

static WalkTrail* walkLcd(Portal* p, char** path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, lcdNodeInfoFn);
}

static Portal* openLcd(Portal* p, Caps caps) {
    return openDev(p, caps);
}

static void closeLcd(Portal* p) {
    UNUSED(p);
    disableLcd(lcdScreen);
    return; /* not sure what this means yet */
}

static ptrdiff_t readLcd(Portal* p, void* buf, size_t size, Offset offset) {
    if (size == 0) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        return readStaticNS(p, lcdSNS, buf, size, offset);
    }

    errno = EPERM;
    return -1; /* not ready to use */
}

static ptrdiff_t writeLcd(Portal* p, void* buf, size_t size, Offset offset) {
    UNUSED(offset);
    if (size == 0) return 0;

    if (!lcdScreen || p->crumb.flags & CRUMB_ISDIR) {
        errno = EPERM;
        return -1;
    }

    NodeInfo ni;
    if (getNodeInfoStaticNS(p, lcdSNS, WalkSelf, &ni) == NULL) {
        errno = ENODEV;
        return -1;
    }

    char *c = NULL;
    int color = -1;
    size_t bytes = 0;
    LcdFidEnt fid = STATICNS_CRUMB_SELF_IDX(p->crumb);
    switch (fid) {
    case FidFg:
    case FidBg:
        color = strtol(buf, &c, 0);
        bytes = (size_t)(c - (char*)buf);
        if (!bytes || bytes > size) {
            errno = EINVAL;
            return -1;
        }

        if (fid == FidFg)
            lcdScreen->colors.fg = color;
        else
            lcdScreen->colors.bg = color;

        break;
    case FidClear:
        if (*(char*)buf == '1') {
            lcdScreen->hw->clear(lcdScreen);
            bytes = 1;
        }
        break;
    case FidBlit:
        lcdScreen->hw->blit(lcdScreen, buf);
        return lcdScreen->fbSize;
    default:
        errno = ENODEV;
        return -1;
    }

    return bytes;
}

static int getInfoLcd(const Portal* p, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, lcdSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

Dev devLcd = {
    .id       = DEV_DEVLCD
,   .name     = "lcd"
,   .power    = powerDev
,   .init     = initDev
,   .reset    = resetLcd
,   .shutdown = shutdownDev
,   .attach   = attachLcd
,   .walk     = walkLcd
,   .create   = createDev
,   .open     = openLcd
,   .close    = closeLcd
,   .remove   = removeDev
,   .getInfo  = getInfoLcd
,   .setInfo  = setInfoDev
,   .read     = readLcd
,   .write    = writeLcd
};
