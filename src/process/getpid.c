#include <manos.h>

Pid getpid(void) {
    return rp ? rp->pid : 0;
}
