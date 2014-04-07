/*
 * Trival ADC device. 
 *
 * Provides 
 *
 * /pot which gives raw potentiometer readings
 * /temp which gives raw temp sensor readings
 */
#include <errno.h>
#include <manos.h>
#include <stdint.h>

#include <arch/k70/derivative.h>

static void initAdcHw(void) {
    SIM_SCGC3 |= SIM_SCGC3_ADC1_MASK;
    ADC1_CFG1  = ADC_CFG1_MODE(0x1); /* bits 12 & 13 */
    ADC1_SC3   = ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(0x3); /* sample average of 32 per cycle */
}

typedef enum {
    AdcPot  = 0x14
,   AdcTemp = 0x1a
} AdcChan;

static int32_t readAdcHw(AdcChan chan) {
    ADC1_SC1A = chan;
    while (!(ADC1_SC1A & ADC_SC1_COCO_MASK))
        ;
    return ADC1_RA;
}

static int32_t readPotAdc(void) {
    return readAdcHw(AdcPot);
}

static int32_t readTempAdc(void) {
    return readAdcHw(AdcTemp);
}

#define NAMESPACE_MAP    \
    X(".", STATICNS_SENTINEL, Dot, CRUMB_ISDIR, 0, 0555, 0) \
    X("pot", FidDot, Pot, CRUMB_ISFILE, 0, 0444, 0) \
    X("temp", FidDot, Temp, CRUMB_ISFILE, 0, 0444, 0)

#define X(p, u, s, t, z, m, c) Fid##s,
typedef enum {
NAMESPACE_MAP
} AdcFidEnt;
#undef X

#define X(p, u, s, t, z, m, c) { p, MKSTATICNS_CRUMB(u, Fid##s, t), z, m, c },
static StaticNS adcSNS[] = {
NAMESPACE_MAP
    { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0, 0}
};
#undef X

static void initAdc(void) {
    initAdcHw();
}

static Portal* attachAdc(char *path) {
    Portal* p = attachDev(DEV_DEVADC, path);
    p->crumb = adcSNS[0].crumb;
    return p;
}

static NodeInfo* adcNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, adcSNS, d, ni);
}

static WalkTrail* walkAdc(Portal* p, char** path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, adcNodeInfoFn);
}

static Portal* openAdc(Portal* p, Caps caps) {
    return openDev(p, caps);
}

static void closeAdc(Portal* p) {
    UNUSED(p);
}

static int getInfoAdc(const Portal* p, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, adcSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

static ptrdiff_t readAdc(Portal* p, void* buf, size_t size, Offset offset) {
    if (size < sizeof(uint32_t)) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        return readStaticNS(p, adcSNS, buf, size, offset);
    }

    AdcFidEnt fid = STATICNS_CRUMB_SELF_IDX(p->crumb);
    switch (fid) {
    case FidPot:
        *(uint32_t*)buf = readPotAdc();
        break;
    case FidTemp:
        *(uint32_t*)buf = readTempAdc();
        break;
    default:
        errno = ENODEV;
        return -1;
    }

    p->offset += sizeof(uint32_t);
    return sizeof(uint32_t);
}

static ptrdiff_t writeAdc(Portal *p, void* buf, size_t size, Offset offset) {
    UNUSED(p);
    UNUSED(buf);
    UNUSED(size);
    UNUSED(offset);
    errno = EPERM;
    return -1; 
}

Dev devAdc = {
    .id       = DEV_DEVADC
,   .name     = "adc"
,   .power    = powerDev
,   .init     = initAdc
,   .reset    = resetDev
,   .shutdown = shutdownDev
,   .attach   = attachAdc
,   .walk     = walkAdc
,   .create   = createDev
,   .open     = openAdc
,   .close    = closeAdc
,   .remove   = removeDev
,   .getInfo  = getInfoAdc
,   .setInfo  = setInfoDev
,   .read     = readAdc
,   .write    = writeAdc
};
