#ifndef MANOS_H
#define MANOS_H

#include "manos/types.h"
#include "manos/api.h"

/* fakeproc stuff */
extern Portal* slash;
extern Portal* dot;

#define MANOS_MAXDEV 1
extern Dev* deviceTable[MANOS_MAXDEV];

#endif /* ! MANOS_H */
