#include <manos.h>
#include <string.h>

char* getCwd(char* buf, size_t n) {
    Portal* p = walk(dot, 0, 0);
    closePortal(p);
    kfree(p);
    memcpy(buf, "fake", strlen("fake") > n ? n - 1 : strlen("fake"));
    *(buf + strlen("fake")) = 0;
    return buf;
}
