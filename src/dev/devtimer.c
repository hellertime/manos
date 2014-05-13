#include <errno.h>
#include <manos.h>
#include <manos/list.h>
#include <string.h>

#ifdef PLATFORM_NICE
#include <sys/time.h>
#include <time.h>
#endif

static unsigned timerSNSCount = 0;
static StaticNS* timerSNS = NULL;

static void resetTimer(void) {
    Timer* timer   = NULL;
    Timer* hpChain = NULL;

    unsigned hpCount = 0;
    for (unsigned i = 0; i < MANOS_MAXTIMER; i++) {
        if ((timer = timerHardwareTable[i]->hotplug()) == NULL)
            continue;

        if (hotpluggedTimers != NULL)
            hpChain->next = timer;
        else
            hotpluggedTimers = timer;

        hpCount++;

        for (hpChain = timer; hpChain->next != NULL; hpChain = hpChain->next)
            hpCount++;
    }

    timerSNSCount = 2 + (2 * hpCount); /* dot, sentinel, timer files */
    timerSNS = syskmalloc(timerSNSCount * sizeof(StaticNS));

    StaticNS* sns = timerSNS;
    strcpy(sns->name, ".");
    sns->crumb.flags = CRUMB_ISDIR | CRUMB_ISSTATIC;
    sns->crumb.fid   = MKSTATICNS_FID(STATICNS_SENTINEL, 0);
    sns->length      = 0;
    sns->mode        = 0555;
    sns->contents    = 0;
    sns++;

    timer = hotpluggedTimers;
    for (unsigned i = 0; i < hpCount; i++) {
        strcpy(sns->name, timer->hw->name);
        sns->crumb.flags = CRUMB_ISFILE | CRUMB_ISSTATIC;
        sns->crumb.fid   = MKSTATICNS_FID(0, i + 1);
        sns->length      = 8;
        sns->mode        = 0666;
        sns->contents    = (char*)timer; /* link back to hardware */
        sns++;

        timer = timer->next;
    }

    sns->name[0]     = 0;
    sns->crumb.flags = CRUMB_ISSTATIC;
    sns->crumb.fid   = MKSTATICNS_FID(STATICNS_SENTINEL, STATICNS_SENTINEL);
    sns->length      = 0;
    sns->mode        = 0;
    sns->contents    = 0;
}

static Portal* attachTimer(char *path) {
    Portal* p = attachDev(DEV_DEVTIMER, path);
    p->crumb  = timerSNS->crumb;
    return p;
}

static NodeInfo* timerNodeInfoFn(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, timerSNS, d, ni);
}

static WalkTrail* walkTimer(Portal* p, char** path, unsigned n) {
    return genericWalk((const Portal*)p, (const char**)path, n, timerNodeInfoFn);
}

static Portal* openTimer(Portal* p, Caps caps) {
    return openDev(p, caps);
}

static void closeTimer(Portal* p) {
    UNUSED(p);
    return;
}

static ptrdiff_t readTimer(Portal* p, void* buf, size_t size, Offset offset) {
    if (size == 0) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        return readStaticNS(p, timerSNS, buf, size, offset);
    }
    
    NodeInfo ni;
    if (getNodeInfoStaticNS(p, timerSNS, WalkSelf, &ni) == NULL) {
        errno = ENODEV;
        return -1;
    }

    Timer* timer = (Timer*)ni.contents;

    if (strcmp(timer->name, "k70PDB0") == 0) {
        errno = EPERM;
        return -1;
    }

    if (offset >= ni.length)
        return 0;

#ifdef PLATFORM_NICE
    struct timeval tv;
    gettimeofday(&tv, NULL);
    timer->timestamp.msecs = ((uint64_t)tv.tv_sec * 1000) + (tv.tv_usec / 1000);
#endif

    size_t bytes = ni.length - offset;
    memcpy(buf, (char*)&timer->timestamp.msecs + offset, bytes);
    return bytes;
}

static ptrdiff_t writeTimer(Portal* p, void* buf, size_t size, Offset offset) {
    UNUSED(offset);
    if (size == 0) return 0;

    if (p->crumb.flags & CRUMB_ISDIR) {
        errno = EPERM;
        return -1;
    }

    NodeInfo ni;
    if (getNodeInfoStaticNS(p, timerSNS, WalkSelf, &ni) == NULL) {
        errno = ENODEV;
        return -1;
    }

    Timer* timer = (Timer*)ni.contents;

    if (strcmp(timer->name, "k70PDB0") == 0) {

        if (timer->oneShotAction) {
            errno = EPERM;
            return -1;
        }

        char duration[21] = {0};
        memcpy(duration, buf, size > 20 ? 20 : size);
        AlarmChain* alarm = syskmalloc0(sizeof *alarm);
        enterCriticalRegion();
        int fd = sysopen("/dev/timer/k70Timer", CAP_READ);
        uint64_t now;
        sysread(fd, &now, sizeof now);
        sysclose(fd);
        alarm->wakeTime = now + atoi(duration);
        alarm->pid = rp ? rp->pid : 0;
        INIT_LIST_HEAD(&alarm->next);
        listAddBefore(&timer->alarms);
        leaveCriticalRegion();
        return (sizeof duration);
    }
    
    char newBuf[8];
    size_t bytes = size > 8 ? 8 : size;
    memcpy(newBuf + (8 - bytes), buf, size);
    memcpy(&timer->timestamp.msecs, newBuf, 8);
    return bytes;
}

static int getInfoTimer(const Portal* p, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, timerSNS, WalkSelf, ni) == NULL ? -1 : 0;
}

static void powerTimer(OnOff onoff) {
    for (Timer* timer = hotpluggedTimers; timer; timer = timer->next) {
        if (timer->hw->power) {
            timer->oneShotAction = 0;
            timer->hw->power(timer, onoff);
            timer->hw->start(timer);
        }
    }
}

Dev devTimer = {
    .id       = DEV_DEVTIMER
,   .name     = "timer"
,   .power    = powerTimer
,   .init     = initDev
,   .reset    = resetTimer
,   .shutdown = shutdownDev
,   .attach   = attachTimer
,   .walk     = walkTimer
,   .create   = createDev
,   .open     = openTimer
,   .close    = closeTimer
,   .remove   = removeDev
,   .getInfo  = getInfoTimer
,   .setInfo  = setInfoDev
,   .read     = readTimer
,   .write    = writeTimer
};
