#include <manos.h>

#ifdef PLATFORM_K70CW

#include <arch/k70/derivative.h>

#ifdef __GNUC__

/* routines based on http://blog.feabhas.com/2013/02/developing-a-generic-hard-fault-handler-for-arm-cortex-m3cortex-m4/ */

static void __attribute__((used)) hardFaultHandlerMain(StackFrame* frame) {
    sysprintln("Oops! 0x%08" PRIx32 "", SCB_HFSR);
    if (SCB_HFSR & SCB_HFSR_FORCED_MASK) {
        sysprintln("*Forced* 0x%08" PRIx32 "", SCB_CFSR);
    }

    sysprintln("r0   = 0x%08" PRIx32 "", frame->a[0]);
    sysprintln("r1   = 0x%08" PRIx32 "", frame->a[1]);
    sysprintln("r2   = 0x%08" PRIx32 "", frame->a[2]);
    sysprintln("r3   = 0x%08" PRIx32 "", frame->a[3]);
    sysprintln("ip   = 0x%08" PRIx32 "", frame->ip);
    sysprintln("lr   = 0x%08" PRIx32 "", frame->lr);
    sysprintln("pc   = 0x%08" PRIx32 "", frame->pc);
    sysprintln("xpsr = 0x%08" PRIx32 "", frame->xpsr);

    __asm volatile("bkpt #01");
    while(1);
}

void __attribute__((naked)) hardFaultHandler(void) {
__asm(
    "tst   lr, #4\n\t"
    "ite   eq\n\t"
    "mrseq r0, msp\n\t"
    "mrsne r0, psp\n\t"
    "bl    hardFaultHandlerMain" /* never returns */
);
}

#else
#error "Unsupported compiler"
#endif

#endif /* PLATFORM_K70CW */
