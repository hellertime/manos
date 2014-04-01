/*
 * dev.c -- default implementation of Dev functions
 */

#include <errno.h>
#include <manos.h>

void initDev(void) {
  /* noop */
}

void shutdownDev(void) {
  /* noop */
}

void resetDev(void) {
  /* noop */
}

/*
 * attachDev :: DevId -> String -> Portal
 *
 * Generic device attach function. Creates a new Portal
 * located at the path.
 */
Portal* attachDev(int device, char *path) {
  UNUSED(path);
  Portal *p = kmalloc(sizeof *p);
  mkPortal(p, fromDeviceId(device));
  p->crumb.flags = CRUMB_ISDIR;
  p->crumb.fid   = 0;
  return p;
}

/*
 * openDev :: Portal -> Caps -> Portal
 *
 * Generic device open.
 */
Portal* openDev(Portal *p, Caps caps) {
  p->offset = 0;
  p->caps = caps;
  p->flags |= PORTAL_ISOPEN;
  return p;
}

/*
 * createDev :: Portal -> String -> Caps -> Mode
 *
 * Generic device create operation.
 */
int createDev(Portal *p, char *name, Caps caps, Mode mode) {
  UNUSED(p);
  UNUSED(name);
  UNUSED(caps);
  UNUSED(mode);
  errno = EPERM;
  return -1;
}

/*
 * removeDev :: Portal -> Err
 * 
 * Generic device remove operation.
 */
int removeDev(Portal *p) {
  UNUSED(p);
  errno = EPERM;
  return -1;
}

/*
 * setInfoDev :: Portal -> DirEnt -> DevInfo -> Err
 * 
 * Generic info set.
 */
int setInfoDev(Portal *p, NodeInfo* info) {
  UNUSED(p);
  UNUSED(info);
  errno = EPERM;
  return -1;
}
