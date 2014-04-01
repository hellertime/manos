#include <errno.h>
#include <manos.h>
#include <stdio.h>
#include <string.h>

/* Test a two-level StaticNS */

/* Rule: siblings to the left, children to the right */
StaticNS twoLevel[] = {
    /* root */
    { "/", MKSTATICNS_CRUMB(STATICNS_SENTINEL, 0, CRUMB_ISDIR), 0, 0 }

    /* children of root. ParentIdx 0 */
,   { "foo", MKSTATICNS_CRUMB(0, 1, CRUMB_ISDIR), 0, 0}
,   { "bar", MKSTATICNS_CRUMB(0, 2, CRUMB_ISDIR), 0, 0}
,   { "baz", MKSTATICNS_CRUMB(0, 3, CRUMB_ISDIR), 0, 0}

    /* children of foo */
,   { "i",   MKSTATICNS_CRUMB(1, 4, 0), 0, 0}
,   { "ii",  MKSTATICNS_CRUMB(1, 5, 0), 0, 0}
,   { "iii", MKSTATICNS_CRUMB(1, 6, 0), 0, 0}

    /* children of bar */
,   { "a", MKSTATICNS_CRUMB(2, 7, 0), 0, 0}
,   { "b", MKSTATICNS_CRUMB(2, 8, 0), 0, 0}
,   { "c", MKSTATICNS_CRUMB(2, 9, 0), 0, 0}

    /* children of baz */
,   { "qux",   MKSTATICNS_CRUMB(3, 10, 0), 0, 0}
,   { "quux",  MKSTATICNS_CRUMB(3, 11, 0), 0, 0}
,   { "quuux", MKSTATICNS_CRUMB(3, 12, 0), 0, 0}

    /* sentinel */
,   { "", MKSTATICNS_SENTINEL_CRUMB, 0, 0 }
};

static NodeInfo* twoLevelNodeInfo(const Portal* p, WalkDirection d, NodeInfo* ni) {
    return getNodeInfoStaticNS(p, twoLevel, d, ni);
}

static StaticNS* toSNS(WalkTrail* t) {
  Crumb c = t->crumbs[t->top - 1]; /* TODO: This is not exactly correct */
  for (unsigned i = 0; i < COUNT_OF(twoLevel); i++) {
    if (twoLevel[i].crumb.fid == c.fid) {
      return &twoLevel[i];
    }
  }

  errno = EINVAL;
  return NULL;
}

int main(int argc, char **argv) {
    Portal root, foo, bar, baz;
    WalkTrail *t;

    mkPortal(&root, 0);
    mkPortal(&foo, 0);
    mkPortal(&bar, 0);
    mkPortal(&baz, 0);

    root.crumb = twoLevel[0].crumb;
    foo.crumb  = twoLevel[1].crumb;
    bar.crumb  = twoLevel[2].crumb;
    baz.crumb  = twoLevel[3].crumb;

    struct T {
        Portal*     portal;
        char*       name;
        char*       expect;
        unsigned    npath;
        const char* path[32];
    } ts[] = {
        { &root, "/.", "/", 1, {"."} }
    ,   { &root, "/..", "/", 1, {".."} }
    ,   { &root, "/foo", "foo", 1, {"foo"} }
    ,   { &root, "/bar", "bar", 1, {"bar"} }
    ,   { &root, "/baz", "baz", 1, {"baz"} }
    ,   { &root, "/foo/i", "i", 2, {"foo", "i"}}
    ,   { &root, "/foo/ii", "ii", 2, {"foo", "ii"}}
    ,   { &root, "/foo/iii", "iii", 2, {"foo", "iii"}}
    ,   { &root, "/bar/a", "a", 2, {"bar", "a"}}
    ,   { &root, "/bar/b", "b", 2, {"bar", "b"}}
    ,   { &root, "/bar/c", "c", 2, {"bar", "c"}}
    ,   { &root, "/baz/qux", "qux", 2, {"baz", "qux"}}
    ,   { &root, "/baz/quux", "quux", 2, {"baz", "quux"}}
    ,   { &root, "/baz/quuux", "quuux", 2, {"baz", "quuux"}}
    ,   { &foo, "./foo/.", "foo", 1, {"."}}
    ,   { &bar, "./foo/.", "bar", 1, {"."}}
    ,   { &baz, "./foo/.", "baz", 1, {"."}}
    ,   { &foo, "./foo/i", "i", 1, {"i"}}
    ,   { &bar, "./foo/b", "b", 1, {"b"}}
    ,   { &baz, "quuux", "quuux", 1, {"quuux"}}
    ,   { &foo, "./foo/..", "/", 1, {".."}}
    ,   { &bar, "./bar/..", "/", 1, {".."}}
    ,   { &baz, "./baz/..", "/", 1, {".."}}
    ,   { &root, "/foo/../bar", "bar", 3, {"foo", "..", "bar"}}
    ,   { &root, "/foo/../bar/./../../baz", "baz", 7, {"foo","..","bar",".","..","..","baz"}}
    };

    for (unsigned i = 0; i < COUNT_OF(ts); i++) {
        printf("Test %04d: %s ", i, ts[i].name);
        fflush(stdout);

        t = genericWalk(ts[i].portal, ts[i].path, ts[i].npath, twoLevelNodeInfo);
        if (!t)
            goto fail;

        if (t->top != t->max)
            goto fail;

        StaticNS* sns = toSNS(t);
        if (!sns)
            goto fail;

        if (strcmp(sns->name, ts[i].expect) != 0) {
            errno = ENODEV;
            goto fail;
        }

        printf("PASS (%s)\n", sns->name);
        goto cleanup;

fail:
        printf("FAIL (%s)\n", strerror(errno));
cleanup:
        if (t)
            freeWalkTrail(t);
    }
    (void)argv;
    (void)argc;
}
