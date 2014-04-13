#include <manos.h> 
extern Dev devRoot;
extern Dev devLed;
extern Dev devSwpb;
extern Dev devUart;
extern Dev devLcd;
extern Dev devAdc;
extern Dev devTimer;

Dev* deviceTable[MANOS_MAXDEV] = {
    &devRoot
,   &devLed
,   &devSwpb
,   &devUart
,   &devLcd
,   &devAdc
,   &devTimer
};

extern UartHW k70UartHW;
extern UartHW niceUartHW;

UartHW* uartHardwareTable[MANOS_MAXUART] = {
    &k70UartHW
,   &niceUartHW
};

extern TimerHW k70TimerHW;

TimerHW* timerHardwareTable[MANOS_MAXTIMER] = {
    &k70TimerHW
};

Proc* u = NULL; 

Uart* consoleUart = NULL;
Uart* hotpluggedUarts = NULL;

Timer* hotpluggedTimers = NULL;

#ifdef PLATFORM_K70CW
extern LcdHw k70LcdHw;
LcdHw* lcdHw = &k70LcdHw;
#else
LcdHw* lcdHw = NULL;
#endif
