#ifndef MANOS_DEV_H
#define MANOS_DEV_H

#include <stddef.h>
#include <stdint.h>

#include <manos/err.h>
#include <manos/portal.h>

typedef char DevId;
typedef unsigned int DevInst;
typedef uint8_t FidType;
typedef unsigned int Mode;
typedef uint32_t Offset;  /* We only need to support filesystems up to 128MB large, so this is a more than fine offset value */
typedef unsigned long Perm;
typedef unsigned long Time;

/*
 * FidTypeFlags
 *
 * The flags tell us about the object identified by .tag in a struct Fid.
 */
typedef enum {
  FID_ISDIR      = 0x80, /* .tag is a directory */
  FID_APPENDONLY = 0x40, /* .tag is an append only file */
  FID_EXCLUSIVE  = 0x20, /* .tag has exclusive access restrictions */
  FID_ISMOUNT    = 0x10, /* .tag exits in the mount table */
  FID_ISFILE     = 0x00  /* .tag is a file */
} FidTypeFlags;

/*
 * Fid = (Int, Int)
 *
 * This structure is used by a device to uniquely identify an object in its
 * local namespace. The first field, 'tag' is an opaque word which uniquely
 * identifies the object with the server. The second field 'type' contains
 * meta information about the object.
 */
struct Fid {
  uint32_t tag;
  FidType type;
};

/*
 * ModeFlags
 *
 * These flags tell us about the object identified by a DevInfo struct.
 * They mirror those of FidTypeFlags in meaning in the high bits, and
 * in the low they are the rwx bits of the file.
 */
typedef enum {
  MODE_ISDIR      = 0x80,
  MODE_APPENDONLY = 0x40,
  MODE_EXCLUSIVE  = 0x20,
  MODE_ISMOUNT    = 0x10,
  MODE_ISREAD     = 0x4,
  MODE_ISWRITE    = 0x2,
  MODE_ISEXEC     = 0x1, /* EXEC, EXPLORE (SEARCH) */
} ModeFlags;

/*
 * DevInfo = (DevId, DevInst, Fid, Mode, Time, Time, Offset, CStr, CStr, CStr)
 *
 * DevInfo plays a simmilar role to a POSIX 'struct stat', a device populates
 * this structure with info about an object in its namespace.
 */
struct DevInfo {
  DevId dev;       /* The device this object belongs to */
  DevInst inst;    /* The device instance of 'dev' */
  struct Fid fid;  /* See: Fid */
  Mode mode;
  Time accessTime;
  Time modTime;
  Offset length;
  char *name;
  char *owner;
  char *group;
};

typedef int FnIndexDirEnt(struct Portal *portal, char *name, struct DirEnt *dirent, size_t size, struct DevInfo *info);

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
   * attach => Dev d :: d -> DevId -> String -> Portal
   *
   * Construct a new Portal, associated with 'name' on the Dev identified by 'dev'.
   * If `IS_ERRPTR(v)` is false the Portal can be used in subsequent calls.
   * Otherwise the pointer is invalid and the error can be obtained by `Err e = FROM_ERRPTR(v);`
   */
  struct Portal* (*attach)(DevId dev, const char* name);


  /*
   * walk => Dev d :: d -> Portal -> Portal -> [String] -> Trail
   *
   * Starting at 'from' walk the device namespace component by component.
   * If the walk ends at components[-1] then the walk was successful and 'to' will contain the new Portal
   * In addition Trail will contain a list of Fids crossed along the way.
   * If the walk failed `IS_ERRPTR(trail)` will be true.
   */
  struct Trail* (*walk)(struct Portal* from, struct Portal *to, const char** components, size_t count);


  /*
   * getInfo => Dev d :: d -> Portal -> DevInfo -> Err
   *
   * Populate a DevInfo structure for the given Portal.
   */
  Err (*getInfo)(struct Portal *p, struct DevInfo *info);

  /*
   * setInfo => Dev d :: d -> Portal -> DevInfo -> Err
   *
   * Update the info of the node at the Portal.
   */
  Err (*setInfo)(struct Portal* p, struct DevInfo *info);


  /*
   * create => Dev d :: d -> Portal -> String -> Mode -> Perm 
   *
   * Create an object associated at the current Portal.
   */
  void (*create)(struct Portal* p, const char *name, Mode mode, Perm perm);


  /*
   * open => Dev d :: d -> Portal -> Mode -> Portal
   *
   * Open the current Vnode for read/write (based on mode).
   */
  struct Portal* (*open)(struct Portal* p, Mode mode);

  /*
   * close => Dev d :: d -> Portal -> ()
   *
   * Close an open Vnode.
   */
  void (*close)(struct Portal* v);

  /*
   * remove => Dev d :: d -> Portal -> ()
   *
   * Remove the name currently associated with the node.
   * It is up to the underlying device to define semantics in the case
   * where multiple Portals are open on the name when it is removed.
   */
  void (*remove)(struct Portal* p);

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

#endif /* ! MANOS_DEV_H */
