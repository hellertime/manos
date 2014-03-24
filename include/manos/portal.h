#ifndef MANOS_PORTAL_H
#define MANOS_PORTAL_H

#include <manos/types.h>

struct Portal* mkPortal(DevId devId);
void freePortal(struct Portal *p);

#endif /* ! MANOS_PORTAL_H */
