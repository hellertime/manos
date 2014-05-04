#include <errno.h>
#include <manos.h>
#include <string.h>
#include <stdlib.h>

#define NAMESPACE_MAP   \
    X(".",          STATICNS_SENTINEL, Dot,        CRUMB_ISDIR,  0, 0555, 0)  \
    X("date",       FidDot,            Date,       CRUMB_ISFILE, 0, 0644, 0)  \
    X("kprint",     FidDot,            KPrint,     CRUMB_ISFILE, 0, 0222, 0)  \
    X("interrupts", FidDot,            Interrupts, CRUMB_ISFILE, 0, 0444, 0)

#define X(p, u, s, t, z, m, c) Fid##s,
typedef enum {
    NAMESPACE_MAP
} DevDevFidEnt;
#undef X

#define X(p, u, s, t, z, m, c) { p, MKSTATICNS_CRUMB(u, Fid##s, t), z, m, c },
static StaticNS devdevSNS[] = {
    NAMESPACE_MAP
    { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0, 0 }
};
#undef X

static Portal* attachDevDev(char* path) {
    Portal* p = attachDev(DEV_DEVDEV, path);
    if (strcmp(path, "date") == 0) {
        p->crumb = devdevSNS[FidDate].crumb;
    } else if (strcmp(path, "kprint") == 0) {
        p->crumb = devdevSNS[FidKPrint].crumb;
    } else if (strcmp(path, "interrupts") == 0) {
        p->crumb = devdevSNS[FidInterrupts].crumb;
    } else {
        p->crumb = devdevSNS[0].crumb;
    }
    return p;
}

static NodeInfo* devdevNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, devdevSNS, d, ni);
}

static WalkTrail* walkDevDev(Portal* p, char** path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, devdevNodeInfoFn);
}

static Portal* openDevDev(Portal* p, Caps caps) {
    return openDev(p, caps);
}

static void closeDevDev(Portal* p) {
    UNUSED(p);
    return;
}

static void readDate(Date* date) {
    memset(date, 0, sizeof *date);

#ifdef PLATFORM_K70CW
    int timer = sysopen("/dev/timer/k70Timer", CAP_READ);
#elif PLATFORM_NICE
    int timer = sysopen("/dev/timer/niceTimer", CAP_READ);
#else
    return;
#endif

    uint64_t msecs;
    sysread(timer, &msecs, sizeof msecs);
    sysclose(timer);

    secondsToDate(msecs / 1000, date);
}

extern long long svcInterruptCount;
extern long long timerInterruptCount;
extern long long pdbInterruptCount;
extern long long systickInterruptCount;
extern long long pendsvInterruptCount;


#define INT_MAP_FMT "%s:\t\t%lld\n"
#define INT_MAP_FMT_OVERHEAD 4     /* 4 overhead bytes in INT_MAP_FMT -- ":\t\t\n" */

static struct IntMap {
    const char*      name;
    const long long* counter;
} intMap[] = {
    { "SVC",     &svcInterruptCount     }
,   { "TIMER",   &timerInterruptCount   }
,   { "PDB",     &pdbInterruptCount     }
,   { "SYSTICK", &systickInterruptCount }
,   { "PENDSV",  &pendsvInterruptCount  }
};

#define INT_MAP_SIZE (5 * 21 + INT_MAP_FMT_OVERHEAD)

static size_t readInterrupts(char* buf, size_t size) {
    char* c = buf;
    size_t bytes = 0;

    for (unsigned i = 0; i < COUNT_OF(intMap); i++) {
        ptrdiff_t nbytes = fmtSnprintf(c, size - bytes, INT_MAP_FMT, intMap[i].name, *intMap[i].counter);
        if (nbytes > 0) {
            bytes += nbytes;
        } else break;
    }

    return bytes;
}

static ptrdiff_t readDevDev(Portal* p, void* buf, size_t size, Offset offset) {
    if (size == 0) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        return readStaticNS(p, devdevSNS, buf, size, offset);
    }

    ptrdiff_t bytes = 0;
    DevDevFidEnt fid = STATICNS_CRUMB_SELF_IDX(p->crumb);
    switch(fid) {
    case FidDate:
        if (size >= sizeof(Date)) {
            readDate((Date*)buf);
            bytes = sizeof(Date);
        } else {
            bytes = 0;
        }
        break;
    case FidInterrupts:
        {
            char fileInfo[INT_MAP_SIZE + 1];
            size_t bytesRead = readInterrupts(fileInfo, INT_MAP_SIZE);
            if (offset < bytesRead) {
                size_t newSize = bytesRead - offset > size ? size : bytesRead - offset;
                memcpy(buf, fileInfo + offset, newSize);
                p->offset += newSize;
                bytes = newSize;
            } else bytes = 0;
        }
        break;
    default:
        errno = EPERM;
        bytes = -1;
    }

    return bytes;
}

static ptrdiff_t writeDate(const char* buf, size_t size) {
    Date d;
    memset(&d, 0, sizeof d);
    int* parts[6] = {&d.year, &d.month, &d.day, &d.hours, &d.minutes, &d.seconds};

    const char* c = buf;
    for (unsigned i = 0; i < COUNT_OF(parts); i++) {
        char *e;
        *parts[i] = strtol(c, &e, 10);
        if (c == e)
            break;

        c = e;
    }

    if ((uintptr_t)(c - buf) > size)
        return 0;

    if (d.month) d.month--;
    if (d.day)   d.day--;

#ifdef PLATFORM_K70CW
    int fd = sysopen("/dev/timer/k70Timer", CAP_WRITE);
    uint64_t msecs = dateToSeconds(&d) * 1000;
    ptrdiff_t bytes = syswrite(fd, &msecs, sizeof msecs);
    sysclose(fd);
#else
    ptrdiff_t bytes = 0;
#endif

    return bytes;
}

static ptrdiff_t writeDevDev(Portal* p, void* buf, size_t size, Offset offset) {
    UNUSED(offset);
    if (size == 0) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        errno = EPERM;
        return -1;
    }

    DevDevFidEnt fid = STATICNS_CRUMB_SELF_IDX(p->crumb);
    switch(fid) {
    case FidDate:
        return writeDate(buf, size);
    case FidKPrint:
        sysnputs(buf, size);
        return size;
    default:
        errno = EPERM;
        return -1;
    }
}

static int getInfoDevDev(const Portal* p , NodeInfo* ni) {
    return getNodeInfoStaticNS(p, devdevSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

Dev devDev = {
    .id       = DEV_DEVDEV
,   .name     = "dev"
,   .power    = powerDev
,   .init     = initDev
,   .reset    = resetDev
,   .shutdown = shutdownDev
,   .attach   = attachDevDev
,   .walk     = walkDevDev
,   .create   = createDev
,   .open     = openDevDev
,   .close    = closeDevDev
,   .remove   = removeDev
,   .getInfo  = getInfoDevDev
,   .setInfo  = setInfoDev
,   .read     = readDevDev
,   .write    = writeDevDev
};
