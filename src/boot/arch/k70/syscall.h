#include <arch/k70/syscall.x>

#define X(c, f, r) MANOS_SYSCALL_##c,
typedef enum {
    SYSCALL_MAP
} SyscallIndex;
#undef X


