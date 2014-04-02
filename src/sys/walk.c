#include <assert.h>
#include <errno.h>
#include <manos.h>

Portal* syswalk(Portal* p, char **path, unsigned n) {
    if (n > 0 && !PORTAL_ISDIR(p)) {
        errno = ENOTDIR;
        return NULL;
    }

    Portal* px = kmallocz(sizeof *px);
    if (!px) {
        return NULL;
    }

    if (clonePortal(p, px) == NULL) {
        kfree(px);
        errno = errno ? errno : ENOTRECOVERABLE;
        return NULL;
    }

    char** subPath = path;
    while (n) {
        WalkTrail* t = deviceTable[p->device]->walk(px, subPath, n);
        subPath += t->top;
        n -= t->top;
        topCrumb(t, &px->crumb);
        if (px->crumb.flags & CRUMB_ISMOUNT) {
            DeviceIndex idx = fromDeviceId(CRUMB_MOUNT_DEVICE_ID(px->crumb));
            closePortal(px);
            kfree(px);
            assert(idx != -1 && "Crumb has an unknown device id");
            px = deviceTable[idx]->attach("");
            freeWalkTrail(t);
            continue;
        }
        
        freeWalkTrail(t);
        break;
    }

    if (n)
        kfree(px);

    return n ? NULL : px;
}
