#ifndef MANOS_API_H
#define MANOS_API_H

#include <stddef.h>

Pid getpid(void);

void* kmalloc(size_t);
void* kmallocz(size_t);
void kfree(void*);

WalkTrail* emptyWalkTrail(unsigned);
void freeWalkTrail(WalkTrail*);
WalkTrail* topCrumb(WalkTrail*, Crumb*);
WalkTrail* popCrumb(WalkTrail*, Crumb*);
WalkTrail* pushCrumb(WalkTrail*, Crumb);
WalkTrail* genericWalk(const Portal*, const char**, unsigned, GetNodeInfoFn);
NodeInfo* getNodeInfoStaticNS(const Portal*, const StaticNS*, WalkDirection, NodeInfo*);

NodeInfo* mkNodeInfo(const Portal*, Crumb, const char*, Offset, Mode, NodeInfo*);

/* can only be used when constructing a static NS */
#define STATICNS_SENTINEL 0xffff
#define MKSTATICNS_CRUMB(p, s, f) { ((f) | CRUMB_ISSTATIC), (((p) & 0xffff) << 16) | ((s) & 0xffff) }
#define MKSTATICNS_SENTINEL_CRUMB MKSTATICNS_CRUMB(STATICNS_SENTINEL, STATICNS_SENTINEL, 0)
#define STATICNS_CRUMB_ISSENTINEL(c) (((c).flags & CRUMB_ISSTATIC) && ((c).fid == STATICNS_SENTINEL))

/* obtain parent and self index values from Crumb */
#define STATICNS_CRUMB_PARENT_IDX(c) ((StaticIndex)(((c).fid >> 16) & 0xffff))
#define STATICNS_CRUMB_SELF_IDX(c) ((StaticIndex)((c).fid & 0xffff))

/* we have a rule that if a crumb has the CRUMB_ISMOUNT flag set than the lower two bytes of the fid
 * are the device id
 */
#define CRUMB_MOUNT_DEVICE_ID(c) ((int)((c).fid & 0xffff))

Portal* mkPortal(Portal*,int);
Portal* clonePortal(const Portal*, Portal*);
Portal* closePortal(Portal*);

#define PORTAL_ISDIR(p) ((p)->crumb.flags & CRUMB_ISDIR)
#define PORTAL_ISSTATICNS(p) ((p)->crumb.flags & CRUMB_ISSTATIC)

void initDev(void);
void shutdownDev(void);
void resetDev(void);
Portal* attachDev(int, char*);
Portal* openDev(Portal*, Caps);
int createDev(Portal*, char*, Caps, Mode);
int removeDev(Portal*);
int setInfoDev(Portal*, NodeInfo*);

DeviceIndex fromDeviceId(DeviceId);
DeviceId toDeviceId(DeviceIndex);

int sysexecv(const char*, char * const []);
int sysopen(const char*, Caps);
void sysclose(int fd);
Portal* syswalk(Portal*, char**, unsigned);

int dirread(int fd, NodeInfo**);
char* getcwd(char*, size_t);

Path* mkPath(const char*);

#define UNUSED(x) (void)(x)
#define COUNT_OF(xs) ((sizeof xs) / (sizeof xs[0]))

#endif /* ! MANOS_API_H */
