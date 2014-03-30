Trail* popCrumb(Trail* t, Crumb *c) {
    if (t->count == 0) {
        errno = ENODATA;
        return &BADPTR;
    }

    *c = t->crumbs[t->count--];
    return t;
}
