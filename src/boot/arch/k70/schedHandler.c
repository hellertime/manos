#include <manos.h>
#include <arch/k70/derivative.h>

#ifdef PLATFORM_K70CW

extern long long systickInterruptCount;
extern long long pendsvInterruptCount;

/** 
 * schedHandler - SysTick and PendSV handler
 */
void __attribute__((naked, used)) schedHandler(void) {
__asm(
    /* On current running task stack */

    "push {r4,r5,r6,r7,r8,r9,r10,r11}\n\t" /* push current Proc stack (hardware already pushed a stack frame) */
    "ldr  r0, [%[shcsr]]\n\t"              /* push interrupt return state (thread or supervisor) */
    "and  r0,r0, %[mask]\n\t"
    "push {r0,lr}\n\t"                     /* push link register, scheduleProc returns here */
    "bl   scheduleProc\n\t"

    /* Switched to new task stack */
    "msr  msp, r0\n\t"                     /* return to main stack always for now */
    "cpsie i\n\t"                          /* scheduleProc leaves us with interrupts disabled. enable them again and take interrupts on the new proc stack */
    "pop {r0}\n\t"                         /* unwind the interrupt return state for the new Proc */
    "ldr r1, [%[shcsr]]\n\t"
    "bic r1, r1, %[mask]\n\t"
    "orr r0, r0, r1\n\t"
    "str r0, [%[shcsr]]\n\t"
    "pop {r4,r5,r6,r7,r8,r9,r10,r11,pc}"   /* unwind Proc stack, and return out of the interrupt, but on the switch Procs stack! */
    :
    : [shcsr] "r" (&SCB_SHCSR), [mask] "I" (SCB_SHCSR_SVCALLACT_MASK)
     );
}

/** 
 * systickHandler - Bookeeping version of systick
 */
void __attribute__((naked)) systickHandler(void) {
    ATOMIC(systickInterruptCount++);
    schedHandler();
}

/**
 * pendsvHandler - Bookeeping version of pendsv
 */
void __attribute__((naked)) pendsvHandler(void) {
    ATOMIC(pendsvInterruptCount++);
    schedHandler();
}

#endif
