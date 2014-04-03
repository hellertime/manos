#include <manos.h>
#include <string.h>

char* getcwd(char* buf, size_t n) {
    Portal* p = syswalk(dot, 0, 0);
    NodeInfo ni;
    deviceTable[p->device]->getInfo(p, &ni);
    kfree(p);
    unsigned x = strlen(ni.name) > n ? n - 1 : strlen(ni.name);
    memcpy(buf, ni.name, x);
    *(buf + x) = 0;
    return buf;
}
