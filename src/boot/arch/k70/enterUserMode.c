#include <manos.h>
#include <arch/k70/derivative.h>

void enterUserMode(void) {
    START_SYSTICK();
#ifdef PLATFORM_K70CW
__asm(
    "mrs    r0, CONTROL\n\t"
    "orr    r0, r0, #1\n\t"
    "msr    CONTROL, r0\n\t"
    "isb    sy"
);
#else
    return;
#endif
}
