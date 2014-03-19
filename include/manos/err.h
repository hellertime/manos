#ifndef MANOS_ERR_H
#define MANOS_ERR_H

#include <manos/err.x>

#define X(c, n, s) E_##c = n,
typedef enum {
  ERROR_XMACRO
} Err;
#undef X

char* fromErr(Err err);

#endif /* ! MANOS_ERR_H */
