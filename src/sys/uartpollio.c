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
    sysnputs(s, strlen(s));
}

void sysnputs(const char* s, size_t n) {
    const char* end = s + n;

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

int sysprintln(const char* fmt, ...) {
    va_list ap;
    static char buf[4096];
    va_start(ap, fmt);
    int ret = fmtVsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    sysputs(buf);
    sysputs("\n");
    return ret + 1;
}

int sysprint(const char* fmt, ...) {
    va_list ap;
    static char buf[4096];
    va_start(ap, fmt);
    int ret = fmtVsnprintf(buf, sizeof buf, fmt, ap);
    va_end(ap);
    sysputs(buf);
    return ret;
}
