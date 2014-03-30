#ifndef MANOS_TYPES_H
#define MANOS_TYPES_H

#include <stdint.h>

typedef unsigned char CrumbFlags;
typedef unsigned long Fid;
typedef unsigned long Mode;
typedef uint32_t Offset;
typedef long Pid;
typedef uint16_t StaticIndex;
typedef unsigned long Time;

#define CRUMB_ISDIR      0x80
#define CRUMB_APPENDONLY 0x40
#define CRUMB_EXCLUSIVE  0x20
#define CRUMB_ISMOUNT    0x10
#define CRUMB_ISSTATIC   0x01
#define CRUMB_ISFILE     0x00

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

typedef struct NodeInfo {
    char*  name;
    Crumb  crumb;
    int    device;
    Mode   mode;
    Time   atime;
    Time   mtime;
    Offset length;
} NodeInfo;

#define PORTAL_STATICNS 0x80

typedef struct Portal {
    int   device;
    int   flags;
    Crumb crumb;
} Portal;

typedef NodeInfo* (*GetNodeInfoFn)(const Portal*, WalkDirection, NodeInfo*);

typedef struct StaticNS {
    const char* name;
    Crumb       crumb;
    Offset      length;
    Mode        mode;
} StaticNS;

#endif /* ! MANOS_TYPES_H */
