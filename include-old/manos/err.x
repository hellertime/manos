#define ERROR_XMACRO \
  X(OK,            0, "Success") \
  X(BADARG,   -10000, "Bad Argument") \
  X(ALLOC,    -10001, "Malloc Error") \
  X(PERM,     -10002, "Permission Error") \
  X(NOFD,     -10003, "No File Descriptors Available") \
  X(NOMOUNT,  -10004, "Not Mounted") \
  X(NODEV,    -10005, "Device Missing") \
  X(NOTFOUND, -10006, "Object Missing") \
  X(SIZE,     -10007, "Illegal Size")
