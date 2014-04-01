#include <errno.h>
#include <manos.h>

WalkTrail* topCrumb(WalkTrail* t, Crumb *c) {
    if (t->top == 0) {
        errno = ENODATA;
        return NULL;
    }

    *c = t->crumbs[t->top - 1];
    return t;
}
