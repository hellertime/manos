#include <assert.h>
#include <manos.h>
#include <stdlib.h>
#include <string.h>

#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR   60
#define HOURS_PER_DAY      24
#define SECONDS_PER_HOUR   (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
#define SECONDS_PER_DAY    (HOURS_PER_DAY * SECONDS_PER_HOUR)

#define IS_LEAP(year) (int)(((year) % 400 == 0) || (((year) % 100 != 0) && ((year) % 4 == 0)))
#define DAYS_IN_YEAR(year) (IS_LEAP(year) ? 366 : 365)

struct Months {
    char*    name;
    unsigned days[2]; /* days[0] is non-leap, days[1] is leap */
} months[] = {
    { "January",    { 31, 31 } }
,   { "February",   { 28, 29 } }
,   { "March",      { 31, 31 } }
,   { "April",      { 30, 30 } }
,   { "May",        { 31, 31 } }
,   { "June",       { 30, 30 } }
,   { "July",       { 31, 31 } }
,   { "August",     { 31, 31 } }
,   { "September",  { 30, 30 } }
,   { "October",    { 31, 31 } } 
,   { "November",   { 30, 30 } }
,   { "December",   { 31, 31 } }
};

struct Date {
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;
};

void secondsToDate(uint64_t seconds, struct Date* date) {
    uint32_t daySeconds = seconds % SECONDS_PER_DAY;
    uint32_t dayOfMonth = seconds / SECONDS_PER_DAY;
    int year = 1970; /* epoch */

    date->seconds = daySeconds % SECONDS_PER_MINUTE;
    date->minutes = (daySeconds % SECONDS_PER_HOUR) / MINUTES_PER_HOUR;
    date->hours   = daySeconds / SECONDS_PER_HOUR;

    while (dayOfMonth >= DAYS_IN_YEAR(year)) {
        dayOfMonth -= DAYS_IN_YEAR(year);
        year++;
    }

    date->year = year;

    date->month = 0;
    while (dayOfMonth >= months[date->month].days[IS_LEAP(year)]) {
        dayOfMonth -= months[date->month].days[IS_LEAP(year)];
        date->month++;
    }

    date->day = dayOfMonth + 1;
}

uint64_t dateToSeconds(struct Date* date) {
    uint64_t seconds = date->seconds + (date->minutes * SECONDS_PER_MINUTE) + (date->hours * SECONDS_PER_HOUR);
    seconds += date->day * SECONDS_PER_DAY;
    for (int i = 0; i < date->month; i++) {
        seconds += months[i].days[IS_LEAP(date->year)] * SECONDS_PER_DAY;
    }

    int year = date->year;
    while (year > 1970) {
        seconds += DAYS_IN_YEAR(year) * SECONDS_PER_DAY;
        year--;
    }

    while (year < 1970) {
        seconds += DAYS_IN_YEAR(year) * SECONDS_PER_DAY;
        year++;
    }

    return seconds;
}

int cmdDate__Main(int argc, char * const argv[]) {
    if (argc > 1) {
        if (argc == 2 || strcmp(argv[1], "--set") != 0) {
            fprint(u->tty, "usage: %s [--set YYYY [MM [DD [HH [MM [SS]]]]]]\n", argv[0]);
            return 1;
        }

        struct Date d;
        memset(&d, 0, sizeof d);
        int* parts[6] = {&d.year, &d.month, &d.day, &d.hours, &d.minutes, &d.seconds};

        for (int i = 2; i < argc; i++) {
             *parts[i - 2] = atoi(argv[i]);
        }

#ifdef PLATFORM_K70CW
        int timer = kopen("/dev/timer/k70Timer", CAP_WRITE);
        uint64_t msecs = dateToSeconds(&d) * 1000;
        kwrite(timer, &msecs, sizeof msecs);
        kclose(timer);
#elif PLATFORM_NICE
        fprint(u->tty, "Cannot set date on this platform.\n");
#else
#error "cmdDate__Main() unsupported platform"
#endif
    }

#ifdef PLATFORM_K70CW
    int timer = kopen("/dev/timer/k70Timer", CAP_READ);
#elif PLATFORM_NICE
    int timer = kopen("/dev/timer/niceTimer", CAP_READ);
#else
#error "cmdDate__Main() unsupprted platform"
#endif

    uint64_t msecs;
    assert(kread(timer, &msecs, sizeof msecs) == 8 && "cmdDate__Main() read error");
    kclose(timer);

    struct Date date;
    secondsToDate(msecs / 1000, &date);
    fprint(u->tty, "%s %d, %d %02d:%02d:%02d\n", months[date.month].name, date.day, date.year, date.hours, date.minutes, date.seconds);
    return 0;
}
