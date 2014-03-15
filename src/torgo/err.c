#include <string.h>

#define SHELL_KEEP_STRERROR
#include <shell/err.h>
#undef SHELL_KEEP_STRERROR

struct ShellErrorMap {
  enum ShellErrors code;
  char *message;
} shellErrorMap[] = {
  { SE_OK,     "Success" },
  { SE_BADARG, "Bad Arguments" },
  { SE_MEMERR, "Malloc Error" },
};

/*
 * shellStrerror :: Int -> String
 *
 * Lookup error code in local map, if its not there pass to system
 * strerror -- assumes a system strerror exists
 */
char* shellStrerror(int err) {
  for (int i = 0; i < sizeof shellErrorMap / sizeof shellErrorMap[0]; i++) {
    if (shellErrorMap[i].code == err)
      return shellErrorMap[i].message;
  }

  return strerror(err);
}
