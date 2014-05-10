#include <manos.h>

#ifdef PLATFORM_K70CW

#include <arch/k70/derivative.h>

#ifdef __GNUC__

/* routines based on http://blog.feabhas.com/2013/02/developing-a-generic-hard-fault-handler-for-arm-cortex-m3cortex-m4/ */

static void __attribute__((used)) hardFaultHandlerMain(StackFrame* frame) {
    volatile uint32_t faulted_r0  = frame->a[0];
    volatile uint32_t faulted_r1  = frame->a[1];
    volatile uint32_t faulted_r2  = frame->a[2];
    volatile uint32_t faulted_r3  = frame->a[3];
    volatile uint32_t faulted_ip  = frame->ip;  /* r12 */
    volatile uint32_t faulted_lr  = frame->lr;
    volatile uint32_t faulted_pc  = frame->pc;
    volatile uint32_t faulted_psr = frame->xpsr;

    volatile uint32_t _CFSR = SCB_CFSR;
    volatile uint32_t _HFSR = SCB_HFSR;
    volatile uint32_t _DFSR = SCB_DFSR;
    volatile uint32_t _AFSR = SCB_AFSR;

    volatile uint32_t _MMFAR = SCB_MMFAR;
    volatile uint32_t _BFAR = SCB_BFAR;

    __asm("bkpt #0\t\n");
    UNUSED(faulted_r0);
    UNUSED(faulted_r1);
    UNUSED(faulted_r2);
    UNUSED(faulted_r3);
    UNUSED(faulted_ip);
    UNUSED(faulted_lr);
    UNUSED(faulted_pc);
    UNUSED(faulted_psr);
    UNUSED(_CFSR);
    UNUSED(_HFSR);
    UNUSED(_AFSR);
    UNUSED(_MMAR);
    UNUSED(_BFAR);
}

void __attribute__((naked)) hardFaultHandler(void) {
__asm(
    "tst   lr, #4\n\t"
    "ite   eq\n\t"
    "mrseq r0, msp\n\t"
    "mrsne r0, psp\n\t"
    "ldr   r1,[r0,#20]\n\t"
    "bl    hardFaultHandlerMain\n\t" 
    "bkpt #0"
    :
    :
    : "r0", "r1"
);
}

#else
#error "Unsupported compiler"
#endif

#endif /* PLATFORM_K70CW */
