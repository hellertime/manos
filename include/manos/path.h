#ifndef MANOS_PATH_H
#define MANOS_PATH_H

/*
 * Path = (String, [Vnode])
 *
 * Path is a structure which identifies a path in the namespace.
 * A path may cross over Vnodes to form its absolute self. 
 * Therefore a history of Vnodes used in this paths construction
 * are maintained in the path itself.
 */

struct Path {
  char *p;    /* canonicalized path */
  struct Vnode **history;
  size_t nhistory;
};

#endif /* ! MANOS_PATH_H */
