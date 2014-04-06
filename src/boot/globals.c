#include <manos.h> 
extern Dev devRoot;
extern Dev devLed;
extern Dev devSwpb;
extern Dev devUart;
extern Dev devLcd;

Dev* deviceTable[MANOS_MAXDEV] = {
    &devRoot
,   &devLed
,   &devSwpb
,   &devUart
,   &devLcd
};

extern UartHW k70UartHW;
extern UartHW niceUartHW;

UartHW* uartHardwareTable[MANOS_MAXUART] = {
    &k70UartHW
,   &niceUartHW
};

Proc* u = NULL; 

Uart* consoleUart = NULL;
Uart* hotpluggedUarts = NULL;

#ifdef PLATFORM_K70CW
LcdHw* lcdHw = &k70LcdHw;
#else
LcdHw* lcdHw = NULL;
#endif
