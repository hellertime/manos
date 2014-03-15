#ifndef MANOS_DEV_H
#define MANOS_DEV_H

typedef char DevId;

/*
 * Dev = (DevId, String
 *     , FnOn, FnReset, FnOff
 *     , FnAttach, FnWalk
 *     , FnGetStat, FnSetStat
 *     , FnOpen, FnClose, FnRemove
 *     , FnRead, FnWrite)
 *
 * Dev is a kernel structure which represents an abstract kernel device.
 * All devices use this common file-oriented protocol, each device has
 * its own implementation of the calls.
 */

struct Dev {
  DevId id;               /* Each device type is assigned an id from the printable ASCII range 0x21-0x7e */
  char *name;             /* Display name of the device type useful for debugging */


  /*
   * on => Dev d :: d -> ()
   *
   * Enable the device to an initial state.
   */
  void (*on)(void);


  /*
   * reset => Dev d :: d -> ()
   *
   * Reset the device to the state it was in when on() was called.
   */
  void (*reset)(void);


  /*
   * off => Dev d :: d -> ()
   *
   * Perform any needed device operations before turning off.
   * The device cannot be used again until 'on' is called.
   */
  void (*off)(void);


  /*
   * attach => Dev d, Vnode v :: d -> DevId -> String -> v
   *
   * Construct a new Vnode v, associated with 'name' on the Dev identified by 'dev'.
   * If `ISVALID_CHAN(v)` is true the Vnode can be used in subsequent calls.
   * Otherwise the pointer is invalid and the error can be obtained by `Err e = GET_ERROR(v);`
   */
  struct Vnode* (*attach)(DevId dev, const char* name);


  /*
   * walk => Dev d, Vnode v1, v2 :: d -> v1 -> [String] -> v2
   *
   * Mutate Vnode v1 -> v2. Starting at 'v1' walk the vnode tree by iterating 'nelems' times over 'elems'.
   * If elems[-1] is found v2 is the new Vnode.
   * If the walk fails `ISVALID_CHAN(v2)` will be false and the error is found in the usual way.
   */
  struct Vnode* (*walk)(const struct Vnode* v, const char** elems, int nelems); 


  /*
   * getStat => Dev d, Vnode v :: d -> v -> Stat
   *
   * Allocate a new Stat structure with the status of the Vnode.
   */
  struct Stat* (*getStat)(const struct Vnode* v);


  /*
   * setStat => Dev d, Vnode v :: d -> v -> Stat -> Err
   *
   * Update the Vnode with the status in Stat. The Err value
   * will either be E_OK or tell a reason why things went wrong.
   */
  Err (*setStat)(const struct Vnode* v, const struct Stat* ent);


  /*
   * create => Dev d, Vnode v1, v2 :: d -> v1 -> String -> Mode -> Perm ->v2 
   *
   * Create a new Vnode at 'name' from the old Vnode. The new entry will be given
   * the appropriate 'mode' and 'perms'. If `IS_FILE(perms) == true` the new Vnode
   * will be open for read/write, otherwise the Vnode is the same as before, unless
   * `mode | VnodeWalk` is true, then this will behave as if a walk was called on 'name'
   */
  struct Vnode* (*create)(struct Vnode* v, const char *name, Mode mode, Perm perm);


  /*
   * open => Dev d, Vnode v1, v2 :: d -> v1 -> Mode -> v2
   *
   * Open the current Vnode for read/write (based on mode).
   */
  struct Vnode* (*open)(struct Vnode* v, Mode mode);

  /*
   * close => Dev d, Vnode v :: d -> v -> ()
   *
   * Close an open Vnode.
   */
  void (*close)(struct Vnode* v);

  /*
   * remove => Dev d, Vnode v :: d -> v -> ()
   *
   * Remove the name currently associated with the node.
   * It is up to the underlying device to define semantics in the case
   * where multiple Vnode are open on the name when it is removed.
   */
  void (*remove)(struct Vnode* v);

  /*
   * read => Dev d, Vnode v :: d -> v -> [Byte] -> Int -> Int -> ErrPtr -> Int
   *
   * Read 'n' bytes into 'buf' starting at 'offset' of the Vnode.
   * 'buf' must be allocated by the caller and sized correctly.
   *
   * If the read fails then the call returns -1 and err is set with the reason.
   * Otherwise the number of bytes read is returned.
   */
  ssize_t (*read)(struct Vnode *v, void *buf, uint32_t n, uint32_t offset, Err *err);

  /*
   * write => Dev d, Vnode v :: d -> v -> [Byte] -> Int -> Int -> ErrPtr -> Int
   *
   * Write 'n' bytes from 'buf' into the Vnode starting at 'offset'.
   * 'buf' must be allocated by the caller and sized to allow a read on 'n' on it.
   *
   * If the write fails then the call resturns -1, and err is set with the reason.
   * Otherwise the number of bytes written is returned.
   */
  ssize_t (*write)(struct Vnode *v, void *buf, uint32_t n, uint32_t offset, Err *err);
};

#endif /* ! MANOS_DEV_H */
