/*
 * dev.c -- default implementation of Dev functions
 */

#include <libc.h>

#include <manos/dev.h>
#include <manos/portal.h>

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
struct Portal* attachDev(DevId devId, char *path) {
  struct Portal *p = mkPortal(devId);
  p->name = strdup(path);
  p->fid.tag  = 0;
  p->fid.type = FID_ISDIR;

  return p;
}

/*
 * openDev :: Portal -> Mode -> Portal
 *
 * Generic device open.
 */
struct Portal* openDev(struct Portal *p, OMode mode) {
  p->offset = 0;
  p->mode = mode;
  p->flags |= PORTAL_ISOPEN;
  return p;
}

/*
 * createDev :: Portal -> String -> OMode -> Perm
 *
 * Generic device create operation.
 */
Err createDev(struct Portal *p, char *name, OMode mode, Perm perm) {
  return E_PERM;
}

/*
 * removeDev :: Portal -> Err
 * 
 * Generic device remove operation.
 */
Err removeDev(struct Portal *p) {
  return E_PERM;
}

/*
 * getInfoDev :: Portal -> DirEnt -> DevInfo -> Err
 * 
 * Looks up the portal in the DirEnt and populates
 * the DevInfo accordingly. Generic routine.
 */
Err getInfoDev(struct Portal *p, struct DirEnt *dent, int count, struct DevInfo *info) {
  for (unsigned i = 0; i < count; i++) {
    if (streq(dent[i].path, p->name)) {
      info->devId = p->devId;
      info->fid = p->fid;
      info->mode = dent[i].mode;
      info->accessTime = 0;
      info->modTime = 0;
      info->name = p->name; /* safe for now, since we're single threaded */
      info->owner = "";
      info->group = "";
      return E_OK;
    }
  }
  
  return E_NOTFOUND;
}

/*
 * setInfoDev :: Portal -> DirEnt -> DevInfo -> Err
 * 
 * Generic info set.
 */
Err setInfoDev(struct Portal *p, struct DevInfo *info) {
  UNUSED(p);
  UNUSED(info);
  return E_PERM;
}