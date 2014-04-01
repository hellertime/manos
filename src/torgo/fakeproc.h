/*
 * Until we have a real process model, this file will holds the bits an pieces to fake it
 */
extern Dev devRoot;

Dev* deviceTable[MANOS_MAXDEV] = {
    &devRoot
};

static Portal* descriptorTable[MAX_FD] = {0};

static void initDeviceTable(void) {
    for (unsigned i = 0; i < COUNT_OF(deviceTable); i++) {
        deviceTable[i]->init();
    }
    UNUSED(descriptorTable);
}

Portal* slash; /* the 'process' root */
Portal* dot;   /* the 'process' cwd */

static int fakeExecv(const char* path, int argc, char **argv) {
    Path* pth = mkPath(path);
    Portal* p = walk(slash, pth->elems, pth->nelems);
    if (p && (p->crumb.flags & CRUMB_ISFILE)) {
        NodeInfo ni;
        getInfo(p, &ni);

        if (ni.mode != 0555)
            goto error;

        p = open(p, CAP_READ);
        char *buf = kmallocz(ni.length+1);
        read(p, buf, ni.length);
        close(p);

        for (unsigned i = 0; i < COUNT_OF(builtinCmds); i++) {
            if (strcmp(builtinCmds[i].cmdName, buf) == 0) {
                return builtinCmds[i].cmd(argc, argv);
            }
        }
    }

error:
    errno = EPERM;
    return -1;
}

#define HAS_FAKEPROC
