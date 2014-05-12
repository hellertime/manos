#include <manos.h>

extern void __manos_exit(void);

void exits(void) {
    __manos_exit();
}
