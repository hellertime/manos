#ifndef MANOS_PATH_H
#define MANOS_PATH_H

/*
 * Path = (String, [Portal])
 *
 * Path is a structure which identifies a path in the namespace.
 * A path may cross over Vnodes to form its absolute self. 
 * Therefore a history of Vnodes used in this paths construction
 * are maintained in the path itself.
 */

struct Path {
  char *p;    /* canonicalized path */
  struct Portal **history;
  size_t nhistory;
};

struct Path* mkPath(char *path);
void freePath(struct Path* path);

#endif /* ! MANOS_PATH_H */
