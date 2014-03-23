#ifndef MANOS_DEV_H
#define MANOS_DEV_H

#include <stddef.h>
#include <stdint.h>

#include <manos/err.h>
#include <manos/portal.h>
#include <manos/types.h>

#define DEV_DEVLED 'l'
#define DEV_DEVSWPB 'B'
#define MAX_DEV 2

/*
 * Dev = (DevId, String
 *     , FnInit, FnReset, FnShutdown,
 *     , FnAttach, FnWalk
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
   * init => Dev d :: d -> ()
   *
   * Enable the device to an initial state.
   */
  void (*init)(void);


  /*
   * reset => Dev d :: d -> ()
   *
   * Reset the device to the state it was in when on() was called.
   */
  void (*reset)(void);


  /*
   * shutdown => Dev d :: d -> ()
   *
   * Perform any needed device operations before turning off.
   * The device cannot be used again until 'init' is called.
   */
  void (*shutdown)(void);


  /*
   * attach => Dev d :: d -> String -> Portal
   *
   * Construct a new Portal, associated with 'name' on the Dev identified by 'dev'.
   * If `IS_ERRPTR(v)` is false the Portal can be used in subsequent calls.
   * Otherwise the pointer is invalid and the error can be obtained by `Err e = FROM_ERRPTR(v);`
   */
  struct Portal* (*attach)(char* name);


  /*
   * create => Dev d :: d -> Portal -> String -> Mode -> Perm  -> Err
   *
   * Create an object associated at the current Portal.
   */
  Err (*create)(struct Portal* p, char *name, OMode mode, Perm perm);


  /*
   * open => Dev d :: d -> Portal -> Mode -> Portal
   *
   * Open the current Vnode for read/write (based on mode).
   */
  struct Portal* (*open)(struct Portal* p, Mode mode);

  /*
   * close => Dev d :: d -> Portal -> ()
   *
   * Close an open Portal
   */
  void (*close)(struct Portal* v);

  /*
   * remove => Dev d :: d -> Portal -> Err
   *
   * Remove the name currently associated with the node.
   * It is up to the underlying device to define semantics in the case
   * where multiple Portals are open on the name when it is removed.
   */
  Err (*remove)(struct Portal* p);

  /*
   * read => Dev d :: d -> Portal -> [Byte] -> Int -> Int -> ErrPtr -> Int
   *
   * Read 'n' bytes into 'buf' starting at 'offset' of the Vnode.
   * 'buf' must be allocated by the caller and sized correctly.
   *
   * If the read fails then the call returns -1 and err is set with the reason.
   * Otherwise the number of bytes read is returned.
   */
  int32_t (*read)(struct Portal *v, void *buf, uint32_t n, Offset offset, Err *err);

  /*
   * write => Dev d :: d -> Portal -> [Byte] -> Int -> Int -> ErrPtr -> Int
   *
   * Write 'n' bytes from 'buf' into the Portal starting at 'offset'.
   * 'buf' must be allocated by the caller and sized to allow a read on 'n' on it.
   *
   * If the write fails then the call resturns -1, and err is set with the reason.
   * Otherwise the number of bytes written is returned.
   */
  int32_t (*write)(struct Portal *p, void *buf, uint32_t n, Offset offset, Err *err);
};

void initDev(void);
void resetDev(void);
void shutdownDev(void);
struct Portal* attachDev(DevId devId, char *path);
struct Portal* openDev(struct Portal *p, OMode mode); 
Err createDev(struct Portal *p, char *name, OMode mode, Perm perm);
Err removeDev(struct Portal *p);

extern struct Dev ledDev;
extern struct Dev swpbDev;

#endif /* ! MANOS_DEV_H */
