/* namespace.c - routines to manage the per-process namespace
 *
 */

#include <libc.h>

/*
 * Mtab = Hash Ment
 *
 * The Mtab maintains a set of links within the namespace.
 * Without the Mtab the namespace would simply be a tree.
 * But the Mtab transforms the namespace into a DAG.
 *
 * Conceptually the Mtab is a map from a namespace Vnode
 * to an ordered list of namespace Vnodes. When traversing
 * the namespace, if one is to walk to a path which crosses
 * over a Vnode which is a key in the Mtab, the walk will
 * proceed to descened not into the Vnode crossed, but instead
 * it will jump to the Vnode value of the map. Actually it will
 * proceed to the Vnode in the list of Vnodes which allows it
 * to continue its walk by having a Vnode representing the next
 * path element.
 */
struct Mtab {
  size_t numBuckets;
  struct Min* buckets[0];
};

/*
 * Min = Vnode MinPtr
 *
 * The Mtab is a hash table which uses chained buckets to handle 
 * collisions. The buckets are called Ments. The 'in' Vnode is
 * the entry point within the namespace for this mount point.
 */
struct Min {
  struct Vnode *vnode;
  struct Mout *out;
  struct Ment *next;
};

/*
 * Mout = Vnode MoutPtrPtr MoutPtr
 *
 * The 'out' side of a mount point. The nodes are doubly linked
 * so that it is possible to reach back to the 'in' side from
 * any of them.
 *
 * 'before' and 'after' are the prev, next links. The names 
 * are mirroring the mount/bind modes which set them.
 */
struct Mout {
  struct Vnode *vnode;
  struct Mout **before;
  struct Mout *after;
};

struct Mtab* mkMtab(size_t buckets) {
  struct Mtab *mt = mallocz(sizeof *mt + (buckets * sizeof(struct Min*)));
  mt->numBuckets = buckets;
  return mt;
}

typedef enum {
  MtabReplace,
  MtabBefore,
  MtabAfter
} MtabMode;

static int hashVnodeMtab(struct Vnode *v, size_t numBuckets) {
  return hashVnode(v) % numBuckets;
}

/*
 * resolveName :: Devtab -> Mtab -> Vnode -> String -> Vnode
 *
 * Returns a Vnode associated with the 'name'. The name is a path
 * or a path fragment in the namespace.
 *
 * If the path is absolute or a Dev path then the resolution follows
 * through the specified device or 'slash', a relative path is fixedup to
 * be rooted to the 'dot' directory which will have an
 * absolute path.
 */
struct Vnode* resolveName(struct Devtab *dtab, struct Mtab *mtab, struct Vnode *slash, struct Vnode *dot, char *name) {
  struct Vnode *res = NULL;

  switch (*name) {
  case '/':
    res = cloneVnode(slash);
    break;
  case '#':
    char dev = *(name + 1);
    res = lookupDevtab(dev)->attach(dev, name);
    break;
  default:
    res = cloneVnode(dot);
    break;
  }

  
}
