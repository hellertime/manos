#ifndef MANOS_H
#define MANOS_H

#include "manos/types.h"
#include "manos/api.h"
#include "arch/mk70f12.h"

#define MANOS_QUANTUM_IN_MILLIS 50

#ifdef NDEBUG
#include <assert.h>
#define ASSERT assert
#else

#ifdef PLATFORM_K70CW
#include <stdlib.h>
#define ASSERT(x) do {                                                                      \
    if (!(x)) {                                                                             \
        sysprintln("Assertion failed: %s (%s: %s: %d)", #x, __FILE__, __func__, __LINE__);  \
        __asm volatile ("bkpt");                                                            \
        abort();                                                                            \
    }                                                                                       \
}while(0)
#else
#include <assert.h>
#define ASSERT assert
#endif

#endif

extern volatile int criticalRegionCount;

#ifdef PLATFORM_K70CW
#define DISABLE_INTERRUPTS() __asm volatile ("cpsid i")
#define ENABLE_INTERRUPTS() __asm volatile ("cpsie i")
#else
#define DISABLE_INTERRUPTS() while(0)
#define ENABLE_INTERRUPTS() while(0)
#endif

#ifdef __GNUC__

static inline void enterCriticalRegion(void) {
    if (criticalRegionCount == 0)
        DISABLE_INTERRUPTS();
    criticalRegionCount++;
}

static inline void leaveCriticalRegion(void) {
    criticalRegionCount--;
    if (criticalRegionCount == 0)
        ENABLE_INTERRUPTS();
}

#else
#error "Critical Region Code Not Supported on this architecture"
#endif

#ifdef PLATFORM_K70CW
#define YIELD() (SCB_ICSR |= SCB_ICSR_PENDSVSET_MASK)
#else
#define YIELD() while(0)
#endif

#ifdef PLATFORM_K70CW
#define START_SYSTICK() (SYST_CSR |= SysTick_CSR_ENABLE_MASK)
#define STOP_SYSTICK() (SYST_CSR &= ~(SysTick_CSR_ENABLE_MASK))
#define RESET_SYSTICK() (SYST_CVR = 0)
#else
#define START_SYSTICK() while(0)
#define STOP_SYSTICK() while(0)
#define RESET_SYSTICK() while(0)
#endif

#define MANOS_MAXPROC 127 /* kernel is proc 0 */

#define MANOS_MAXDEV 8
extern Dev* deviceTable[MANOS_MAXDEV];

#define MANOS_MAXUART 2
extern UartHW* uartHardwareTable[MANOS_MAXUART];

#define MANOS_MAXTIMER 2
extern TimerHW* timerHardwareTable[MANOS_MAXTIMER];

extern Timer* hotpluggedTimers;

extern Lock malLock;
extern Lock freelistLock;
extern ListHead procFreelist;
extern Lock runQLock;
extern ListHead procRunQ;
extern Ref nextPid;
extern Proc** procTable;
extern Proc* rp; /* always the current running process */

extern Uart* consoleUart; /* UART connected to the console */
extern Uart* hotpluggedUarts; /* populated with the installed UARTS */

extern LcdHw* lcdHw;     /* attached LCD hardware */

#define INIT_LOCK(lock) do {      \
    (lock)->locked = 0;           \
    INIT_LIST_HEAD(&((lock)->q)); \
}while(0)

#define INIT_REF(ref) do {      \
    (ref)->count = 0;           \
    INIT_LOCK(&((ref)->lock));  \
}while(0)

#endif /* ! MANOS_H */
