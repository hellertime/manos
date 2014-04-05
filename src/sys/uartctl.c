#include <manos.h>
#include <stdlib.h>

static unsigned parseCmds(const char *s, const char* buf[], unsigned n) {
    const char* c;
    unsigned i;
    for (c = s, i = 0; i < n && *c && *c != '\n';) {
        switch (*c) {
            case ' ':
            case '\t':
                c++;
                break;
            default:
                buf[i] = c;
                while (*c != ' ' && *c != '\t')
                    c++;
                i++;
                break;
        }
    }

    return i;
}

/*
 * Small command language for Uarts
 *
 * - Line based.
 * - Commands are <type><arg>
 * - Multiple commands can be whitespace separated
 *
 * Commands:
 *
 * type: b (baud) arg: int
 * type: l (bits) arg: int
 */
int sysuartctl(Uart* uart, const char *cmd) {
    const char* cmds[2];

    unsigned n = parseCmds(cmd, cmds, COUNT_OF(cmds));
    for (unsigned i = 0; i < n; i++) {
        unsigned arg = atoi(cmds[i] + 1);
        switch (*cmds[i]) {
            case 'B':
            case 'b':
                if (uart->hw->baud(uart, arg) < 0)
                    return -1;
                break;
            case 'L':
            case 'l':
                if (uart->hw->bits(uart, arg) < 0)
                    return -1;
                break;
        }
    }
    return 0;
}
