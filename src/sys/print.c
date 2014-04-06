#include <errno.h>
#include <manos.h>
#include <stdarg.h>
#include <string.h>

int fputchar(int fd, char c) {
    return syswrite(fd, &c, 1);
}

int fputstrn(int fd, const char* s, size_t n) {
    return syswrite(fd, (char*)s, n);
}

int fputstr(int fd, const char* s) {
    return fputstrn(fd, s, strlen(s));
}

int vfprint(int fd, const char* fmt, va_list ap) {
    static char buf[4096];
    int ret = fmtVsnprintf(buf, sizeof buf, fmt, ap);
    return fputstrn(fd, buf, ret);
}

int fprint(int fd, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprint(fd, fmt, ap);
    va_end(ap);
    return ret;
}

int fprintln(int fd, const char* fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    int ret = vfprint(fd, fmt, ap);
    va_end(ap);
    return ret + fputstrn(fd, "\n", 1);
}
