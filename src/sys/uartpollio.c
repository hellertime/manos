#include <errno.h>
#include <manos.h>
#include <string.h>
#include <stdarg.h>

#define UARTGETC()  consoleUart->hw->getc(consoleUart)
#define UARTPUTC(c) consoleUart->hw->putc(consoleUart, (c))

int sysgetchar(void) {
    if (consoleUart == NULL || consoleUart->hw->getc == NULL) {
        errno = ENODEV;
        return -1;
    }

    return UARTGETC();
}

void sysputchar(int c) {
    if (consoleUart == NULL || consoleUart->hw->putc == NULL) {
        errno = ENODEV;
        return;
    }

    UARTPUTC(c);
}

void sysputs(const char* s) {
    const char* end = s + strlen(s);

    if (consoleUart == NULL || consoleUart->hw->putc == NULL) {
        errno = ENODEV;
        return;
    }

    while (s < end) {
        if (*s == '\n')
            UARTPUTC('\r');

        UARTPUTC(*s++);
    }
}

#include <stdio.h>

int sysprintln(const char* fmt, ...) {
    static char buf[4096];
    va_list ap;
    va_start(ap, fmt);
    int ret = vsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    sysputs(buf);
    sysputs("\n");
    return ret;
}
