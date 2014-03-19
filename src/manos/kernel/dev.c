/*
 * dev.c -- default implementation of Dev functions
 */

#include <manos/dev.h>
#include <manos/portal.h>

/*
 * indexDirEntDev :: Portal -> String -> [DirEnt] -> DirPtr -> Int
 *
 */
/* int indexDirEntDev( */

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
  struct Portal *p = mkPortal();
  p->fid.ent  = 0;
  p->fid.type = FIDISDIR;
  p->dev = lookupDevNumDevId(devId);

  path = path ? path : "";

  int size = 3 + strlen(path); /* '#' <devId:char> <path:string> NUL */
  char *buf = malloc(size);
  snprintf(buf, size, "#%c%s", (char)devId, path);

  p->path = mkPath(buf);
  free(buf);

  return p;
}
