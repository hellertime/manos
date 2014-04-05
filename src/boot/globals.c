#include <manos.h>

extern Dev devRoot;
extern Dev devLed;
extern Dev devSwpb;
extern Dev devUart;

Dev* deviceTable[MANOS_MAXDEV] = {
    &devRoot
,   &devLed
,   &devSwpb
,   &devUart
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

