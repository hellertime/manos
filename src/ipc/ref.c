#include <manos.h>

int incRef(Ref* ref) {
    lock(&ref->lock);
    int x = ++ref->count;
    unlock(&ref->lock);
    return x;
}

int decRef(Ref* ref) {
    lock(&ref->lock);
    int x = -- ref->count;
    unlock(&ref->lock);
    return x;
}
