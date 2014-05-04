#include <errno.h>
#include <manos.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

char intToHexLC[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

/* converts i into a string representation */
static int uintToStr(char* buf, size_t n, int hex, uint32_t i) {
    if (n == 0) return 0;

    if (i == 0) {
        *(buf + n - 1) = '0';
        return 1;
    }

    int base = hex ? 16 : 10;
    char* c = buf + n - 1;
    int x = n;

    while (x && i) {
        uint32_t r = i % base;
        i /= base;

        if (hex) { 
            *c = intToHexLC[r];
        } else {
            *c = r + '0';
        }
        c--;
        x--;
    }
    
    return n - x;
}

/* although we aren't likely to encounter 64-bit integers it costs little to
 * allow for such sizing. Max value in a 64-bit integer will be 20 digits long
 * so our buffer for storing integers will be size accordingly
 */
#define INTBUF_SIZE 20

/*
 * (worst printf implementation ever)
 * printf style formatting to a buffer of size n. This is the backbone of
 * the printf style functions.
 *
 * Format language follows from printf:
 *
 * Holes start with a '%', "%%" is a literal '%'
 *
 * Language is:
 *
 * %[flag][width][.precision][type]
 *
 * flag can be:
 *
 * '0' -- zero pad output
 *
 * width can be:
 *
 * any int
 *
 * precision can be
 *
 * any int
 *
 * type can be:
 *
 * d -- int
 * u -- unsigned
 * l -- long
 * ul -- unsigned long
 * x -- int, printed in hex
 * s -- NULL terminated string
 */
static ptrdiff_t fmtVsnprintfInternal(char buf[], size_t n, int useN, const char* fmt, va_list ap) {
    char intBuf[INTBUF_SIZE + 1] = {0};
    const char* c = fmt;
    char* p = buf;

    while (n && *c) {
        const char* x = c;

        /* scan for format delimiter */
        while ((n || !useN) && *x && *x != '%') {
            *p++ = *x;
            x++;
            n--;
        }

        c = x;

        if (*x == '%') { 
            if (*(x + 1) == '%') {
                *p++ = '%';
                c = x + 2;
                n--;
                continue;
            }
            x++;
            /* 1: handle flags */
           switch (*x) {
           case '0':
               memset(intBuf, '0', INTBUF_SIZE);
               x++;
               break;
           }
           c = x;

           /* 2: handle width */
           unsigned width = 0;
           while (*x >= '0' && *x <= '9')
               x++;

           width = atoi(c);
           c = x;

           int hasPrecision = 0;
           unsigned precision = 0;
           /* 2a: handle precision */
           if (*x == '.') {
               hasPrecision = 1;
               x++;
               c = x;

               while (*x >= '0' && *x <= '9')
                   x++;

               precision = atoi(c);
           }

           c = x;

           int longModifiers = 0;
           size_t bytes = 0;
           char* s;
           /* 3: handle type */
restartOnLongModifier:
           switch (*x) {
           case 'l':
               x++;
               longModifiers++;
               goto restartOnLongModifier;
           case 'd':
           case 'u':
               bytes = uintToStr(intBuf, INTBUF_SIZE, 0, va_arg(ap, uint32_t));
               if (bytes < width) bytes = width;
               if (useN && bytes > n) bytes = n;
               memcpy(p, intBuf + INTBUF_SIZE - bytes, bytes);
               n -= bytes;
               p += bytes;
               c = x + 1;
               break;
           case 'x':
               bytes = uintToStr(intBuf, INTBUF_SIZE, 1, va_arg(ap, uint32_t));
               if (bytes < width) bytes = width;
               if (useN && bytes > n) bytes = n;
               memcpy(p, intBuf + INTBUF_SIZE - bytes, bytes);
               n -= bytes;
               p += bytes;
               c = x + 1;
               break;
           case 's':
               s = va_arg(ap, char*);
               bytes = strlen(s);
               if (hasPrecision && precision < bytes) bytes = precision;
               if (width > bytes) {
                   width -= bytes;
                   while (width && (n || !useN)) {
                       *p = ' ';
                       p++;
                       n--;
                       width--;
                   }
               }
               if (useN && !n) break;
               if (useN && bytes > n) bytes = n;
               memcpy(p, s, bytes);
               n -= bytes;
               p += bytes;
               c = x + 1;
               break;
           }
        }
    }
    *p = 0;
    return p - buf;
}

ptrdiff_t fmtVsnprintf(char buf[], size_t n, const char* fmt, va_list ap) {
    return fmtVsnprintfInternal(buf, n, 1, fmt, ap);
}

ptrdiff_t fmtSnprintf(char buf[], size_t n, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ptrdiff_t bytes = fmtVsnprintf(buf, n, fmt, ap);
    va_end(ap);
    return bytes;
}

ptrdiff_t fmtSprintf(char buf[], const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    ptrdiff_t bytes = fmtVsnprintfInternal(buf, strlen(buf), 0, fmt, ap);
    va_end(ap);
    return bytes;
}
