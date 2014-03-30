/*
 * Create an empty WalkTrail capable of holding 'n' crumbs.
 *
 * 'n' has been chosen as 'unsigned' since it represents
 * the depth of the walk down a tree (technically its the
 * depth + 1).
 */
WalkTrail* emptyWalkTrail(unsigned n) {
    WalkTrail* t = mallocz(sizeof *t + (sizeof Crumb * n));
    if (!t) {
      errno = ENOMEM;
      return NULL;
    }

    return t;
}