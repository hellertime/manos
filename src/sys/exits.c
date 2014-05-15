#include <manos.h>

extern void __manos_exit(void);

void exits(void) {
    recycleProc(rp);
    __manos_exit();
}
