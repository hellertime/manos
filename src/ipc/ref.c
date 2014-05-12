#include <manos.h>

int getReg(Ref* ref) {
    syslock(&ref->lock);
    int x = ref->count;
    sysunlock(&ref->lock);
    return x;
}

int incRef(Ref* ref) {
    syslock(&ref->lock);
    int x = ++ref->count;
    sysunlock(&ref->lock);
    return x;
}

int decRef(Ref* ref) {
    syslock(&ref->lock);
    int x = -- ref->count;
    sysunlock(&ref->lock);
    return x;
}
