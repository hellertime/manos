#include <manos.h>
#include <arch/k70/derivative.h>

#define SVC_MAX_PRIORITY 15

void svcInit(uint8_t priority) {
   if (priority > SVC_MAX_PRIORITY)
       return;

   /* This sets the SVC call priority which is vector 11 in the NVIC */
   SCB_SHPR2 = (SCB_SHPR2 & ~SCB_SHPR2_PRI_11_MASK) /* clear out the old priority */
             | SCB_SHPR2_PRI_11(priority << 4);     /* only the high order bits are used, so push our value up in there */
}
