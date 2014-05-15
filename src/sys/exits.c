#include <manos.h>

extern void __manos_exit(void);

void exits(void) {
    abortProc(rp);
    __manos_exit();
}
