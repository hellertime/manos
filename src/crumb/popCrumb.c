#include <errno.h>
#include <manos.h>
#include <stddef.h>

WalkTrail* popCrumb(WalkTrail* t, Crumb *c) {
    if (t->top == 0) {
        errno = ENODATA;
        return NULL;
    }

    *c = t->crumbs[t->top--];
    return t;
}
