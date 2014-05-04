#include <manos.h>
#include <arch/k70/derivative.h>

void schedInit(int quantumMillis, uint8_t priority) {
    SCB_SHPR3 = (SCB_SHPR3 & ~(SCB_SHPR3_PRI_14_MASK & SCB_SHPR3_PRI_15_MASK)) /* clear the old priority for SysTick and PendSV */
              | SCB_SHPR3_PRI_14(priority << 4)                                /* set the high bits of the PendSV interrupt */
              | SCB_SHPR3_PRI_15(priority << 4);                               /* set the high bits of the SysTick interrupt */

    SYST_CSR |= SysTick_CSR_CLKSOURCE_MASK;                                              /* enable the SysTick clock source to use the processor clock (120MHz) */
    SYST_RVR = SysTick_RVR_RELOAD(quantumMillis * MANOS_ARCH_K70_CYCLES_PER_MILLIS - 1); /* setup the SysTick quantum scaled to to cycle count of 8 1/3 nanos - counting starts at 1 */
}
