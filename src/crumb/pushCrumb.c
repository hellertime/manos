WalkTrail* pushCrumb(WalkTrail* t, Crumb* c) {
    if (t->top >= t->max) {
        errno = ENOBUFS;
        return NULL;
    }

    t->crumbs[t->top++] = *c;
    return t;
}
