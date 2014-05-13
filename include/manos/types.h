#ifndef MANOS_TYPES_H
#define MANOS_TYPES_H

#include <stddef.h>
#include <stdint.h>

typedef unsigned char CrumbFlags;
typedef unsigned long Fid;
typedef unsigned long Mode;
typedef unsigned long Caps;
typedef uint32_t Offset;
typedef long Pid;
typedef uint16_t StaticIndex;
typedef unsigned long Time;
typedef int DeviceIndex;
typedef int DeviceId;
typedef int OnOff;

#define DEV_DEVROOT  'R'
#define DEV_DEVLED   'l'
#define DEV_DEVSWPB  'B'
#define DEV_DEVUART  'u'
#define DEV_DEVLCD   'D'
#define DEV_DEVADC   'A'
#define DEV_DEVTIMER 'T'
#define DEV_DEVDEV   '='

#define CAP_READ      0
#define CAP_WRITE     1
#define CAP_READWRITE 2

#define CRUMB_ISDIR      0x80
#define CRUMB_APPENDONLY 0x40
#define CRUMB_EXCLUSIVE  0x20
#define CRUMB_ISMOUNT    0x10
#define CRUMB_ISSTATIC   0x02
#define CRUMB_ISFILE     0x01

/**
 * struct ListHead - a generic doubly linked list
 * @prev:            previous node in the list
 * @next:            ext node in the list
 */
typedef struct ListHead {
    struct ListHead* prev;
    struct ListHead* next;
} ListHead;

typedef struct Lock {
    int             locked;
    struct ListHead q;
} Lock;

typedef struct Ref {
    Lock lock;
    int  count;
} Ref;

typedef struct FifoQ {
    int    isEmpty;
    size_t readOffset;
    size_t writeOffset;
    size_t size;
    char   buf[];
} FifoQ;

typedef struct HeapQ {
    size_t   n;
    size_t   size;
    uint32_t buf[];
} HeapQ;

/*
 * A Crumb is a namespace object identifier and metadata
 */
typedef struct Crumb {
    CrumbFlags flags;
    Fid        fid;
} Crumb;

/*
 * A WalkTrail encodes the path traveled on a walk
 *
 * A stack is used to store the Crumbs
 */
typedef struct WalkTrail {
    unsigned max;
    unsigned top;
    Crumb    crumbs[];
} WalkTrail;

typedef enum {
  WalkUp,
  WalkDown,
  WalkPrev,
  WalkNext,
  WalkSelf
} WalkDirection;

typedef struct Path {
    char **elems;    /* buffer of strings (join with '/' to form path */
    unsigned nelems; /* number of strings in elems */
    unsigned nbuf;   /* number of strings in elems backed by buf */
    char buf[];       /* buf contains nbuf strings, which the first nbuf pointers in elems point into */
}Path;

typedef struct NodeInfo {
    char*       name;
    Crumb       crumb;
    DeviceIndex device;
    Mode        mode;
    Time        atime;
    Time        mtime;
    Offset      length;
    char*       contents; /* only used by static ns */
} NodeInfo;

#define PORTAL_STATICNS 0x80
#define PORTAL_ISOPEN   0x01

typedef struct Portal {
    DeviceIndex device;
    int         flags;
    Crumb       crumb;
    Caps        caps;
    Offset      offset;
} Portal;

typedef NodeInfo* (*GetNodeInfoFn)(const Portal*, WalkDirection, NodeInfo*);

#define MANOS_MAXNAME 256

typedef struct StaticNS {
    char   name[MANOS_MAXNAME];
    Crumb  crumb;
    Offset length;
    Mode   mode;
    char*  contents;
} StaticNS;

typedef struct Dev {
    DeviceId id;
    char *name;

    void (*power)(OnOff);
    void (*init)(void);
    void (*reset)(void);
    void (*shutdown)(void);
    Portal* (*attach)(char*);
    WalkTrail* (*walk)(Portal*, char**, unsigned);
    int (*create)(Portal*, char*, Caps, Mode);
    Portal* (*open)(Portal*, Caps);
    void (*close)(Portal*);
    int (*remove)(Portal*);
    int (*getInfo)(const Portal*, NodeInfo*);
    int (*setInfo)(Portal*, NodeInfo*);
    ptrdiff_t (*read)(Portal*, void*, size_t, Offset);
    ptrdiff_t (*write)(Portal*, void*, size_t, Offset);
} Dev;

typedef struct Uart Uart;
typedef struct UartHW UartHW;

struct Uart {
    void*    regs;
    char*    name;
    uint32_t clock;
    UartHW*  hw;
    unsigned baud;
    int      bits;
    FifoQ*   inQ;
    FifoQ*   outQ;
    int      enabled;
    int      console;
    Uart*    next;
};

struct UartHW {
    char* name;
    Uart* (*hotplug)(void);
    void (*enable)(Uart*);    
    void (*disable)(Uart*);
    void (*power)(Uart*, int);
    int (*baud)(Uart*, unsigned);
    int (*bits)(Uart*, int);
    char (*getc)(Uart*);
    void (*putc)(Uart*, char);
};

#define MANOS_MAXFD 1024
#define MANOS_MAXSIGPENDING 32

typedef enum {
    ProcDead,
    ProcSpawning,
    ProcReady,
    ProcRunning,
    ProcWaiting,
    ProcStopped,
} ProcState;

/**
 * struct ProcGroup - a process group
 *
 * @memberCount: reference count on group membership
 * @pgid:        process group id, takes the id of the first process in the group
 */
typedef struct ProcGroup {
    Ref      memberCount;
    int      pgid;
} ProcGroup;

/**
 * enum ProcSig - process signals
 */
typedef enum ProcSig {
    SigAbort    = 0x00000001
,   SigContinue = 0x00000002
,   SigStop     = 0x00000004
} ProcSig;

/**
 * struct Proc - a process thread
 *
 * @pid:             process id
 * @ppid:            parent pid
 * @tty:             process console
 * @argv:            argv of the Proc
 * @state:           Proc scheduler state
 * @descriptorTable: open file descriptors
 * @slash:           /
 * @dot:             ./
 * @waitQ:           queue of Procs waiting on this 
 * @nextWaitQ:       list head to add to other wait queues
 * @nextRunQ:        list head to add to procRunQ
 * @nextFreelist:    list head to add to procFreelist (TODO: consolidate these)
 * @pgrp:            ProcGroup pointer
 * @canary1:         stack canary at the top of the stack
 * @canart2:         stack canary at the bottom of the stack
 * @stack:           process stack
 * @sp:              stack pointer
 * @sigPending:      flag to note a posted signal
 * @signalQ:         signal heap
 */
typedef struct Proc {
    Pid        pid;
    Pid        ppid;
    int        tty;
    char**     argv;
    ProcState  state;
    Portal*    descriptorTable[MANOS_MAXFD];
    Portal*    slash;
    Portal*    dot;
    ListHead   waitQ;
    ListHead   nextWaitQ;
    ListHead   nextRunQ;
    ListHead   nextFreelist;
    ProcGroup* pgrp;
    /* TODO:  track memory allocations with asym-dll, release proc memory on exit, use allocation as storage for linkage */
    uint64_t*  canary1;
    uint64_t*  canary2;
    uint32_t*  stack;
    uint32_t   sp;
    uint32_t   sigPending;
    uint32_t   sigMask;
} Proc;

typedef struct StackFrame {
    /* argument 1 / integer result / scratch register */
    /* argument 2 / scratch register */
    /* argument 3 / scratch register */
    /* argument 4 / scratch register */
    int a[4]; /* stored as a vector, and passed en-mass */
    int ip;   /* scratch register / new-sb in inter-link-unit calls */
    int lr;   /* link address / scratch register */
    int pc;   /* program counter */
    int xpsr; /* combined APSR / IPSR / EPSR bits -- does the hardware push this ? */
} StackFrame;

typedef struct LcdHw LcdHw;

typedef struct Lcd {
    void*    regs;
    char*    name;
    LcdHw*   hw;
    uint32_t fbSize;
    uint32_t colorDepth;
    struct {
    uint32_t bg;
    uint32_t fg;
    } colors;
    uint32_t consX;
    uint32_t consY;
} Lcd;

struct LcdHw {
    char* name;
    Lcd* (*hotplug)(void);
    void (*enable)(Lcd*);
    void (*disable)(Lcd*);
    void (*clear)(Lcd*);
    void (*blit)(Lcd*, char*);
    void (*scroll)(Lcd*);
    void (*putc)(Lcd*,int);
};

typedef struct Timestamp {
    int64_t msecs;
} Timestamp;

typedef struct TimerHW TimerHW;

typedef struct Timer {
    void*         regs;
    char*         name;
    int           clock;
    int           psd;
    int           mod;
    void          (*oneShotAction)(void);
    Timestamp     timestamp;
    TimerHW*      hw;
    struct Timer* next;
} Timer;

struct TimerHW {
    char*  name;
    Timer* (*hotplug)(void);
    void   (*disable)(Timer*);
    void   (*power)(Timer*, int);
    void   (*reset)(Timer*);
    void   (*start)(Timer*);
    void   (*stop)(Timer*);
    void   (*clear)(Timer*);
};

typedef struct Date {
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;
} Date;

#endif /* ! MANOS_TYPES_H */
