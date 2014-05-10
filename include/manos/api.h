#ifndef MANOS_API_H
#define MANOS_API_H

#include <stddef.h>
#include <stdarg.h>
#include <stdint.h>

Pid getpid(void);
Proc* newProc(void);
void recycleProc(Proc*);

void* kmalloc(size_t);
void kfree(void*);

void* syskmalloc(size_t);
void* syskmalloc0(size_t);
void syskfree(void*);

void syswaitpid(int);

WalkTrail* emptyWalkTrail(unsigned);
void freeWalkTrail(WalkTrail*);
WalkTrail* topCrumb(WalkTrail*, Crumb*);
WalkTrail* popCrumb(WalkTrail*, Crumb*);
WalkTrail* pushCrumb(WalkTrail*, Crumb);
WalkTrail* genericWalk(const Portal*, const char**, unsigned, GetNodeInfoFn);
NodeInfo* getNodeInfoStaticNS(const Portal*, const StaticNS*, WalkDirection, NodeInfo*);
ptrdiff_t readStaticNS(Portal*, const StaticNS*, void*, size_t, Offset);

NodeInfo* mkNodeInfo(const Portal*, Crumb, const char*, Offset, Mode, NodeInfo*);

/* can only be used when constructing a static NS */
#define STATICNS_SENTINEL 0xffff
#define MKSTATICNS_FID(p, s) ((((p) & 0xfff) << 16) | ((s) & 0xffff))
#define MKSTATICNS_CRUMB(p, s, f) { ((f) | CRUMB_ISSTATIC), MKSTATICNS_FID(p,s) }
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

void powerDev(OnOff);
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
int sysgetInfoFd(int fd, NodeInfo*);
int sysopen(const char*, Caps);
void sysclose(int);
ptrdiff_t sysread(int, void*, size_t);
ptrdiff_t syswrite(int, void*, size_t);
int sysuartctl(Uart*, const char*);
Portal* syswalk(Portal*, char**, unsigned);

int systrylock(Lock*);
void syslock(Lock*);
void sysunlock(Lock*);

int dirread(int fd, NodeInfo**);
char* getcwd(char*, size_t);

Path* mkPath(const char*);
    
void niceConsole(void);
void k70Console(void);

int sysgetchar(void);
void sysputchar(int);
void sysnputs(const char*, size_t);
void sysputs(const char*);
int sysprintln(const char*, ...);
int sysprint(const char*, ...);

int fputchar(int, char);
int fputstrn(int, const char*, size_t);
int fputstr(int, const char*);
int vfprint(int, const char*, va_list);
int fprint(int, const char*, ...);
int fprintln(int, const char*, ...);

ptrdiff_t fmtVsnprintf(char [], size_t, const char*, va_list);
ptrdiff_t fmtSnprintf(char [], size_t, const char*, ...);
ptrdiff_t fmtSprintf(char [], const char*, ...);

FifoQ* newFifoQ(size_t);
FifoQ* clearFifoQ(FifoQ*);
int    isFullFifoQ(FifoQ*);
int    enqueueFifoQ(FifoQ*, char);
int    dequeueFifoQ(FifoQ*, char*);

void secondsToDate(uint64_t, struct Date* date);
uint64_t dateToSeconds(const struct Date*);

#define UNUSED(x) (void)(x)
#define USED UNUSED
#define COUNT_OF(xs) ((sizeof xs) / (sizeof xs[0]))

void svcInit(uint8_t);
void svcHandler(void);
void hardFaultHandler(void);
void enableNvicIrq(unsigned, uint8_t);
void schedInit(int, uint8_t);

/*
 * ipc
 */
int trylock(Lock*);
void lock(Lock*);
void unlock(Lock*);

int incRef(Ref*);
int decRef(Ref*);

void waitpid(int);

/*
 * System calls
 */
int kexec(const char*, char * const []);
void kclose(int);
int kfstat(int, NodeInfo*);
int kopen(const char*, Caps);
ptrdiff_t kread(int, void*, size_t);
ptrdiff_t kwrite(int, void*, size_t);

#define ATOMIC(expr) do {   \
    DISABLE_INTERRUPTS();   \
    (expr);                 \
    ENABLE_INTERRUPTS();    \
} while (0)

#endif /* ! MANOS_API_H */
