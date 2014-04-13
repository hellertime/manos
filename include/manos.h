#ifndef MANOS_H
#define MANOS_H

#include "manos/types.h"
#include "manos/api.h"
#include "arch/mk70f12.h"

#define MANOS_MAXDEV 7
extern Dev* deviceTable[MANOS_MAXDEV];

#define MANOS_MAXUART 2
extern UartHW* uartHardwareTable[MANOS_MAXUART];

#define MANOS_MAXTIMER 1
extern TimerHW* timerHardwareTable[MANOS_MAXTIMER];

extern Timer* hotpluggedTimers;

extern Proc* u; /* always the current user pointer */

extern Uart* consoleUart; /* UART connected to the console */
extern Uart* hotpluggedUarts; /* populated with the installed UARTS */

extern LcdHw* lcdHw;     /* attached LCD hardware */

#endif /* ! MANOS_H */
