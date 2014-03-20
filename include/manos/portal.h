#ifndef MANOS_PORTAL_H
#define MANOS_PORTAL_H

#include <manos/types.h>

struct Portal {
  DevId devId;
  char *name;
  struct Fid fid;
  Offset offset;
  OMode mode;
  int flags;
};

struct Portal* mkPortal(DevId devId);

#endif /* ! MANOS_PORTAL_H */
