#include <string.h>

#include <libc.h>

#include <manos/err.h>
#include <manos/err.x>

#define X(c, n, s) { E_##c, s },
static struct {
  Err code;
  const char *msg;
} errorMap[] = {
  ERROR_XMACRO
};
#undef X

/*
 * fromErr :: Err -> String
 *
 * Lookup error code in local map and return its string value.
 */
const char* fromErr(Err err) {
  for (int i = 0; i < COUNT_OF(errorMap); i++) {
    if (errorMap[i].code == err) {
      return errorMap[i].msg;
    }
  }
  return "Unknown";
}
