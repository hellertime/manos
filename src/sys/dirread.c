#include <errno.h>
#include <manos.h>
#include <string.h>

/* This controls how many directories are read at a time ... */
#define DIRREAD_N 2

/* This controls how many bytes we read from the Portal. On a Directory
 * a device will always write integral directory entries.
 */
#define DIRREAD_READBUF_MAX 4096
/*
 * dirread allocates buf into two parts
 * the first is an array for NodeInfo*
 * the seconds is an array of NodeInfo
 */
int dirread(int fd, NodeInfo** buf) {
    Portal* p = u->descriptorTable[fd];
    if (!p) {
        errno = EBADF;
        return -1;
    }

    if (!(p->crumb.flags & CRUMB_ISDIR)) {
        errno = ENOTDIR;
        return -1;
    }

    char readBuf[DIRREAD_READBUF_MAX];
    int dirs = deviceTable[p->device]->read(p, readBuf, DIRREAD_READBUF_MAX, p->offset);

    if (dirs < 1) return dirs; /* -1 error, 0 eof */

    char* backing = kmalloc(sizeof **buf * DIRREAD_N);
    NodeInfo *ni = (NodeInfo*)backing;

    Crumb fake = MKSTATICNS_SENTINEL_CRUMB;
    char *c = readBuf;
    for (int i = 0; i < dirs; i++) {
        mkNodeInfo(p, fake, c, 0, 0, &ni[i]); /* TODO format is incorrect! */
        c += strlen(c) + 1;
    }
    *buf = (NodeInfo*)backing;
    return dirs;
}
