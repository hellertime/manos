Trail* pushCrumb(Trail* t, Crumb* c) {
    if (t->count >= t->size) {
        errno = ENOBUFS;
        return &BADPTR;
    }

    t->crumbs[t->count++] = *c;
    return t;
}
