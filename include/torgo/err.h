#ifndef SHELL_ERR_H
#define SHELL_ERR_H

#ifndef SHELL_KEEP_STRERROR
#define strerror(x) shellStrerror(x)
#endif

char* shellStrerror(int err);

enum ShellErrors {
  SE_OK = 0,
  SE_BADARG = -16000,
  SE_LASTERR = -16001,
  SE_MEMERR = -16002,
  SE_RESERVED = -16999
};

#endif /* ! SHELL_ERR_H */
