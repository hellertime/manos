#include <manos.h>

Crumb* mkCrumb(Crumb* c, Fid f, CrumbFlags flags) {
  c->fid   = f;
  c->flags = flags;
  return c;
}
