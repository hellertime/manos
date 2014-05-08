#include <manos.h>
#include <string.h>

Path* mkPath(const char *path) {
    Path* p = syskmalloc(sizeof *p + strlen(path) + 1);
    if (!p) return NULL;

    char *c = (char*)path;
    unsigned nelems = 0;

    if (*c == '/') {
        nelems++;
        c++; /* skip leading '/' */
    }

    /* copy the path, replace '/' with '\0' */
    memcpy(p->buf, c, strlen(c));
    c = p->buf;

    while(*c) {
        if (*c == '/') {
            nelems++;
            *c = 0;
        }
        c++;
    }

    if (nelems == 0 && strlen(path))
        nelems = 1;

    /* allocate char* buffer */
    p->elems = syskmalloc(sizeof c * nelems);
    p->nelems = nelems;
    p->nbuf = nelems;
    
    c = p->buf;
    for (unsigned i = 0; i < nelems; i++) {
        p->elems[i] = (char*)c;
        c += strlen(c) + 1;
    }

    return p;
}
