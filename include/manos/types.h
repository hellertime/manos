#ifndef MANOS_TYPES_H
#define MANOS_TYPES_H

#include <stdint.h>

typedef char DevId;
typedef unsigned int DevInst;
typedef uint8_t FidType;
typedef unsigned int Mode;  /* File mode bits */
typedef unsigned int OMode; /* Open mode flags */
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
 * OpenModeFlags
 *
 * The flags are passed to a call to open() to indicate desired operation.
 */
typedef enum {
  OMODE_READ      = 0,
  OMODE_WRITE     = 1,
  OMODE_READWRITE = 2,
} OpenModeFlags;

/*
 * OpenModeModifierFlags
 *
 * The flags are OR'd with OpenModeFlags for further specifics.
 */
typedef enum {
  OMOD_TRUNCATE = 16,
} OpenModeModifierFlags;

/*
 * PortalFlags
 *
 * Verious condition flags about a portal
 */
typedef enum {
  PORTAL_ISOPEN = 0x1
} PortalFlags;

#endif /* ! MANOS_TYPES_H */
