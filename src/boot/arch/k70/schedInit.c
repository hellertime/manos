#include <manos.h>

void schedInit(uint8_t priority) {
    SCB_SHPR3 = (SCB_SHPR3 & ~(SCB_SHPR3_PRI_14_MASK & SCB_SHPR3_PRI_15_MASK)) /* clear the old priority for SysTick and PendSV */
              | SCB_SHPR3_PRI_14(priority << 4)                                /* set the high bits of the PendSV interrupt */
              | SCB_SHPR3_PRI_15(priority << 4);                               /* set the high bits of the SysTick interrupt */
}
