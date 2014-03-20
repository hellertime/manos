/*
 * portal.c 
 */

#include <libc.h>

#include <manos/types.h>
#include <manos/portal.h>

struct Portal* mkPortal(DevId devId) {
  struct Portal *p = malloc(sizeof *p);
  p->devId = devId;
  return p;
}
