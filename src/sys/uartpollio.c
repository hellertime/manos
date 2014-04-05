#include <errno.h>
#include <manos.h>
#include <string.h>

#define GETC()  consoleUart->hw->getc(consoleUart)
#define PUTC(c) consoleUart->hw->putc(consoleUart, (c))

int sysgetchar(void) {
    if (consoleUart == NULL || consoleUart->hw->getc == NULL) {
        errno = ENODEV;
        return -1;
    }

    return GETC();
}

void sysputchar(int c) {
    if (consoleUart == NULL || consoleUart->hw->putc == NULL) {
        errno = ENODEV;
        return;
    }

    PUTC(c);
}

void sysputs(const char* s) {
    const char* end = s + strlen(s);

    if (consoleUart == NULL || consoleUart->hw->putc == NULL) {
        errno = ENODEV;
        return;
    }

    while (s < end) {
        if (*s == '\n')
            PUTC('\r');

        PUTC(*s++);
    }
}
