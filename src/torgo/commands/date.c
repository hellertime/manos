#include <manos.h>
#include <string.h>

char* months[] = { "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December" };

int cmdDate__Main(int argc, char * const argv[]) {
    if (argc > 1) {
        if (argc == 2 || strcmp(argv[1], "--set") != 0) {
            fprint(rp->tty, "usage: %s [--set \"YYYY [MM [DD [HH [MM [SS]]]]]\"]\n", argv[0]);
            return 1;
        }

#ifdef PLATFORM_K70CW
        int fd = kopen("/dev/date", CAP_WRITE);
        kwrite(fd, argv[2], strlen(argv[2]));
        kclose(fd);
#elif PLATFORM_NICE
        fprint(rp->tty, "Cannot set date on this platform.\n");
#else
#error "cmdDate__Main() unsupported platform"
#endif
    }

    Date date;
    int fd = kopen("/dev/date", CAP_READ);
    kread(fd, &date, sizeof date);
    kclose(fd);

    fprint(rp->tty, "%s %d, %d %02d:%02d:%02d\n", months[date.month], date.day, date.year, date.hours, date.minutes, date.seconds);
    return 0;
}
