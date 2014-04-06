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

#define DEV_DEVROOT 'R'
#define DEV_DEVLED  'l'
#define DEV_DEVSWPB 'B'
#define DEV_DEVUART 'u'

#define CAP_READ      0
#define CAP_WRITE     1
#define CAP_READWRITE 2

#define CRUMB_ISDIR      0x80
#define CRUMB_APPENDONLY 0x40
#define CRUMB_EXCLUSIVE  0x20
#define CRUMB_ISMOUNT    0x10
#define CRUMB_ISSTATIC   0x02
#define CRUMB_ISFILE     0x01

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

#define MANOS_MAXFD 4096

typedef struct Proc {
    Pid     pid;
    int     tty;
    Portal* descriptorTable[MANOS_MAXFD];
    Portal* slash;
    Portal* dot;
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

#endif /* ! MANOS_TYPES_H */
