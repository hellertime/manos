#include <manos.h> 
#include <manos/list.h>

extern Dev devRoot;
extern Dev devLed;
extern Dev devSwpb;
extern Dev devUart;
extern Dev devLcd;
extern Dev devAdc;
extern Dev devTimer;
extern Dev devDev;

long long svcInterruptCount     = 0;
long long timerInterruptCount   = 0;
long long pdbInterruptCount     = 0;
long long systickInterruptCount = 0;
long long pendsvInterruptCount  = 0;

Dev* deviceTable[MANOS_MAXDEV] = {
    &devRoot
,   &devLed
,   &devSwpb
,   &devUart
,   &devLcd
,   &devAdc
,   &devTimer
,   &devDev
};

extern UartHW k70UartHW;
extern UartHW niceUartHW;

UartHW* uartHardwareTable[MANOS_MAXUART] = {
    &k70UartHW
,   &niceUartHW
};

extern TimerHW k70TimerHW;
extern TimerHW niceTimerHW;

TimerHW* timerHardwareTable[MANOS_MAXTIMER] = {
    &k70TimerHW
,   &niceTimerHW
};

Lock malLock;

Lock freelistLock;
LIST_HEAD(procFreelist);

Lock runQLock;
LIST_HEAD(procRunQ);

Ref nextPid;

Proc* rp = NULL; 

Uart* consoleUart = NULL;
Uart* hotpluggedUarts = NULL;

Timer* hotpluggedTimers = NULL;

#ifdef PLATFORM_K70CW
extern LcdHw k70LcdHw;
LcdHw* lcdHw = &k70LcdHw;
#else
LcdHw* lcdHw = NULL;
#endif
