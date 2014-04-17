#include <manos.h>
#include <stdlib.h>
#include <string.h>

#define EPOCH_YEAR 1970

#define SECONDS_PER_MINUTE 60
#define MINUTES_PER_HOUR   60
#define HOURS_PER_DAY      24
#define SECONDS_PER_HOUR   (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
#define SECONDS_PER_DAY    (HOURS_PER_DAY * SECONDS_PER_HOUR)

#define IS_LEAP(year) (int)(((year) % 400 == 0) || (((year) % 100 != 0) && ((year) % 4 == 0)))
#define DAYS_IN_YEAR(year) (IS_LEAP((year)) ? 366 : 365)

unsigned monthDays[2][12] = {
    /*j   f   m   a   m   j   j   a   s   o   n  d*/
    {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
,   {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 }
};

const char* monthStrings[] = {
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

const char* monthStrs[] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

void secondsToDate(uint64_t seconds, Date* date) {
    uint32_t daySeconds = seconds % SECONDS_PER_DAY;
    uint32_t days       = seconds / SECONDS_PER_DAY;
    int year = EPOCH_YEAR;

    date->seconds = daySeconds % SECONDS_PER_MINUTE;
    date->minutes = (daySeconds % SECONDS_PER_HOUR) / MINUTES_PER_HOUR;
    date->hours   = daySeconds / SECONDS_PER_HOUR;

    while (days >= DAYS_IN_YEAR(year)) {
        days -= DAYS_IN_YEAR(year);
        year++;
    }

    date->year = year;

    date->month = 0;
    while (days >= monthDays[IS_LEAP(year)][date->month]) {
        days -= monthDays[IS_LEAP(year)][date->month];
        date->month++;
    }

    date->day = days + 1;
}

uint64_t dateToSeconds(const Date* date) {
    uint64_t seconds = date->seconds + (date->minutes * SECONDS_PER_MINUTE) + (date->hours * SECONDS_PER_HOUR);
    seconds += date->day * SECONDS_PER_DAY;
    for (int i = 0; i < date->month; i++) {
        seconds += monthDays[IS_LEAP(date->year)][i] * SECONDS_PER_DAY;
    }

    int year = date->year;
    while (year > EPOCH_YEAR) {
        seconds += DAYS_IN_YEAR(year) * SECONDS_PER_DAY;
        year--;
    }

    while (year < EPOCH_YEAR) {
        seconds += DAYS_IN_YEAR(year) * SECONDS_PER_DAY;
        year++;
    }

    return seconds;
}
