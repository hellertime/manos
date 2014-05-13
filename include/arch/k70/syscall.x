/*
 * SYSCALL_MAP (x-macro)
 *
 * Constant, Function, isVoid?
 */
#define SYSCALL_MAP              \
    X(EXEC,       exec,      0)  \
    X(CLOSE,      close,     1)  \
    X(FSTAT,      fstat,     0)  \
    X(OPEN,       open,      0)  \
    X(READ,       read,      0)  \
    X(WRITE,      write,     0)  \
    X(TRYLOCK,    trylock,   0)  \
    X(LOCK,       lock,      1)  \
    X(UNLOCK,     unlock,    1)  \
    X(WAITPID,    waitpid,   1)  \
    X(EXITS,      _exits,    1)  \
    X(POSTSIGNAL, postsignal, 0) \
    X(SLEEP,      sleep,     0)


