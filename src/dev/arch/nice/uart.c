#include <manos.h>

#ifdef PLATFORM_NICE /* dont leak stdio */
#include <stdio.h>
#endif

extern UartHW niceUartHW;

Uart niceUart[] = {
{    .name    = "stdio"
,    .clock   = 0
,    .hw      = &niceUartHW
,    .next    = 0
}
};

static void niceUartPower(Uart* uart, int onoff) {
    UNUSED(uart);
    UNUSED(onoff);
}

static Uart* niceUartHotplug(void) {
#ifdef PLATFORM_NICE
    return niceUart;
#else
    return NULL;
#endif
}

static void niceUartEnable(Uart* uart) {
    UNUSED(uart);
}

static void niceUartDisable(Uart* uart) {
    UNUSED(uart);
}

static int niceUartBits(Uart* uart, int bits) {
    UNUSED(uart);
    UNUSED(bits);
    return 0;
}

static int niceUartBaud(Uart* uart, unsigned baud) {
    UNUSED(uart);
    UNUSED(baud);
    return 0;
}

static char niceUartGetc(Uart* uart) {
    UNUSED(uart);
#ifdef PLATFORM_NICE
    return getchar();
#else
    return 0;
#endif
}

static void niceUartPutc(Uart* uart, char c) {
    UNUSED(uart);
#ifdef PLATFORM_NICE
    putchar(c);
#else
    UNUSED(c);
#endif
}

UartHW niceUartHW = {
    .name    = "stdio"
,   .hotplug = niceUartHotplug
,   .enable  = niceUartEnable
,   .disable = niceUartDisable
,   .power   = niceUartPower
,   .baud    = niceUartBaud
,   .bits    = niceUartBits
,   .getc    = niceUartGetc
,   .putc    = niceUartPutc
};

void niceConsole(void) {
    Uart* uart = &niceUart[0];

    uart->hw->enable(uart);
    sysuartctl(uart, "b9600 l8 pn s1");
    consoleUart = uart;
    uart->console =1;
}
