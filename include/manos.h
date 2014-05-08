#ifndef MANOS_H
#define MANOS_H

#include "manos/types.h"
#include "manos/api.h"
#include "arch/mk70f12.h"

#define MANOS_QUANTUM_IN_MILLIS 50

#ifdef PLATFORM_K70CW
#define DISABLE_INTERRUPTS() __asm("cpsid i")
#define ENABLE_INTERRUPTS() __asm("cpsie i")
#else
#define DISABLE_INTERRUPTS() while(0)
#define ENABLE_INTERRUPTS() while(0)
#endif

#ifdef PLATFORM_K70CW
#define YIELD() (SCB_ICSR |= SCB_ICSR_PENDSVSET_MASK)
#else
#define YIELD() while(0)
#endif

#define MANOS_MAXPROC 127 /* kernel is proc 0 */

#define MANOS_MAXDEV 8
extern Dev* deviceTable[MANOS_MAXDEV];

#define MANOS_MAXUART 2
extern UartHW* uartHardwareTable[MANOS_MAXUART];

#define MANOS_MAXTIMER 2
extern TimerHW* timerHardwareTable[MANOS_MAXTIMER];

extern Timer* hotpluggedTimers;

extern Lock freelistLock;
extern ListHead procFreelist;
extern ListHead procRunQ;
extern Proc* rp; /* always the current running process */

extern Uart* consoleUart; /* UART connected to the console */
extern Uart* hotpluggedUarts; /* populated with the installed UARTS */

extern LcdHw* lcdHw;     /* attached LCD hardware */

#endif /* ! MANOS_H */
