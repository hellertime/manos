#ifndef MANOS_H
#define MANOS_H

#include "manos/types.h"
#include "manos/api.h"

/* fakeproc stuff */
extern Portal* slash;
extern Portal* dot;

#define MANOS_MAXDEV 1
extern Dev* deviceTable[MANOS_MAXDEV];

#define MANOS_MAXFD 4096
extern Portal* descriptorTable[MANOS_MAXFD];

#endif /* ! MANOS_H */
