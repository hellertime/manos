#ifndef MANOS_VNODE_H
#define MANOS_VNODE_H

typedef int VnodeId

/*
 * Vnode = (VnodeId, DevId, Path, Offset, Mode, Perms)
 *
 * Vnode is a kernel structure which represents a communication point
 * in the kernel namespace. Essentially it is a bi-directional channel
 * between a process and a device.
 *
 */

struct Vnode {
  VnodeId id;
  DevId   devId;
  Path    path;
  off_t   offset;
  Mode    mode;
  Perms   perms;
};

#endif /* ! MANOS_VNODE_H */
