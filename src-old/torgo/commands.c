/**
 * commands.c
 *
 * Builtin shell commands
 */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
/*
#include <sys/time.h>
#include <time.h>
*/

#include <manos.h>

#include <torgo/commands.h>

int cmdDate(int argc, char *argv[]);
int cmdDate2(int argc, char *argv[]);
int cmdEcho(int argc, char *argv[]);
int cmdExit(int argc, char *argv[]);
int cmdFree(int argc, char *argv[]);
int cmdHelp(int argc, char *argv[]);
int cmdMalloc(int argc, char *argv[]);
int cmdMemmap(int argc, char *argv[]);

/*
 * Commands must be mapped into this table
 * to be callable.
 */
struct CmdTable builtinCmds[] = {
                   { "date"     , cmdDate }
                  ,{ "d2"       , cmdDate2 }
                  ,{ "echo"     , cmdEcho }
                  ,{ "exit"     , cmdExit }
                  ,{ "help"     , cmdHelp }
                  ,{ "memorymap", cmdMemmap }
                  };

int numBuiltinCmds = sizeof builtinCmds / sizeof builtinCmds[0];

/*
 * convertToTimeval :: String -> Timeval
 *
 * Construct a timeval from a string in the form
 * <unix timestamp>[.<microseconds>]
 */
#if 0 /* DISASBLE */
int convertToTimeval(const char *str, struct timeval *tv) {
  const char *start = str;
  const char *c;
  time_t tv_sec = 0;
  suseconds_t tv_usec = 0;
  int sign = 1;

  if (!start) goto fail;

  if (*start == '-') {
    sign = -1;
    start++;
  }

  c = start;

  while (*(c + 1)) c++;

  int scale = 1;
  while (c > start && *c != '.') {
    if (*c < '0' || *c > '9') goto fail;

    tv_usec += (*c - '0') * scale;
    scale *= 10;
    c--;
  }

  if (c == start) {
    if (*c == '.') goto fail;

    tv_usec += (*c - '0') * scale;
    tv_sec = tv_usec;
    tv_usec = 0;
  } else if (*c == '.') {
    c--;
    scale = 1;
    while (c > start) {
      if (*c < '0' || *c > 9) goto fail;

      tv_sec += (*c - '0') * scale;
      scale *= 10;
      c--;
    }

    tv_sec += (*c - '0') *  scale;
  } else {
    goto fail;
  }

  tv->tv_sec = tv_sec * sign;
  tv->tv_usec = tv_usec;

  return 0;

fail:
  return -1;
}
#endif /* DISABLE */
#define GREG_AVG_DAYS_PER_YEAR 365.2425

int cmdDate2(int argc, char **argv) {
    UNUSED(argc);
    UNUSED(argv);
#if 0 /* DISABLE */
  struct timeval tv;

  if (argc > 1)
    convertToTimeval(argv[1], &tv);
  else
    tv.tv_sec = 0;
    /* gettimeofday(&tv, NULL); */

  time_t unixTime = tv.tv_sec;

  /* 1. Ignoring leap seconds there are 24 * 60 * 60 seconds per day, or 86400 */
  int days = unixTime / 86400;
  int seconds = unixTime % 60;
  int minutes = (unixTime / 60) % 60;
  int hours = (unixTime / 3600) % 24;

  /* 2. Unix Time counts from Jan 1, 1970 00:00:00 GMT, and is using the Gregorian calendar */
  int year = 1970 + (int)(days / GREG_AVG_DAYS_PER_YEAR);

  fprintf(stdout, "Unix Time %ld\n", unixTime);
  fprintf(stdout, "Day Span %d\n", days);
  fprintf(stdout, "Year Span %d\n", (int)(days / GREG_AVG_DAYS_PER_YEAR));
  fprintf(stdout, "Day of Year %d\n", days - ((int)(days / GREG_AVG_DAYS_PER_YEAR) * days));

  fprintf(stdout, "%s %02d, %d %02d:%02d:%02d.%ld\n", "MONTH", -1, year, hours, minutes, seconds, tv.tv_usec);

#endif /* DISSBLE */
  return 0;
}

/*
 * Numeric constants to make the date manipulation code clearer.
 */
#define UNIX_EPOCH_YEAR        1970
#define GMT_OFFSET             -5 * 60 * 60
#define LEAP_YEAR_CYCLE        4
#define SECONDS_PER_MINUTE     60
#define MINUTES_PER_HOUR       60
#define SECONDS_PER_HOUR       (MINUTES_PER_HOUR * SECONDS_PER_MINUTE)
#define HOURS_PER_DAY          24
#define HOURS_PER_YEAR         8765.81
#define DAYS_PER_NON_LEAP_YEAR 365
#define SECONDS_PER_DAY        (HOURS_PER_DAY * MINUTES_PER_HOUR * SECONDS_PER_MINUTE)

#define IS_LEAP(x) (int)((x % 400 == 0) || ((x % 100 != 0) && (x % 4 == 0)))

/*
 * cmdDate :: StdArgs -> String
 *
 * Formats a date-time string from the current time using an algorithm
 * adapted from the URL: http://forums.mrplc.com/index.php?showtopic=13294&st=0&p=65248&#entry65248
 */

int cmdDate(int argc, char *argv[]) {
    UNUSED(argc);
    UNUSED(argv);
#if 0 /* DISABLE */
  struct timeval now;

  if (argc > 2) {
    fprintf(stdout, "usage: date [TIMESTAMP]\n");
    return E_BADARG;
  }

  errno = 0;
  if (argc > 1) {
    if (convertToTimeval(argv[1], &now) != 0) {
      fprintf(stdout, "error: %s\n", fromErr(errno));
      return errno;
    }
  } else {
    if (gettimeofday(&now, NULL) != 0) {
      fprintf(stdout, "error: %s\n", fromErr(errno));
      return errno;
    }
  }

  time_t theTime = now.tv_sec;

  /* 1. Deconstruct the timestamp */

  int years = theTime / (HOURS_PER_YEAR * SECONDS_PER_HOUR);
  int year = years + UNIX_EPOCH_YEAR;
  int leapDays = years / LEAP_YEAR_CYCLE;
  int days = (theTime / SECONDS_PER_DAY);
  int daySeconds = theTime - (days * SECONDS_PER_DAY);
  int hours = daySeconds / SECONDS_PER_HOUR;
  int min = (daySeconds - (hours * SECONDS_PER_HOUR)) / MINUTES_PER_HOUR;
  int sec = (daySeconds - (hours * SECONDS_PER_HOUR)) - (min * SECONDS_PER_MINUTE);

  struct Months {
    char *name;
    int days[2]; /* days[0] is non-leap, days[1] is leap */
  } months[] = {
    { "January"   , { 31, 31 } },
    { "February"  , { 28, 29 } },
    { "March"     , { 31, 31 } },
    { "April"     , { 30, 30 } },
    { "May"       , { 31, 31 } },
    { "June"      , { 30, 30 } },
    { "July"      , { 31, 31 } },
    { "August"    , { 31, 31 } },
    { "September" , { 30, 30 } },
    { "October"   , { 31, 31 } },
    { "November"  , { 30, 30 } },
    { "December"  , { 31, 31 } }
  };

  char *month = "Movember";

  /* 2. Identify the day of the month and the month name for the date */

  int monthDay = (days - leapDays) % DAYS_PER_NON_LEAP_YEAR;

  /* 3. Correct for negative values */

  if (monthDay < 0) {
    monthDay += DAYS_PER_NON_LEAP_YEAR;
    year--;
  }

  if (hours < 0) {
    hours += HOURS_PER_DAY;
    monthDay--;
  }

  if (min < 0) {
    min += MINUTES_PER_HOUR;
    hours--;
  }

  if (sec < 0) {
    sec += SECONDS_PER_MINUTE;
    min--;
  }

  /* 4. Do the actual lookup */

  for (int i = 0; i < sizeof months / sizeof months[0]; i++) {
    if (monthDay < months[i].days[IS_LEAP(year)]) {
      month = months[i].name;
      monthDay++;
      break;
    }

    monthDay -= months[i].days[IS_LEAP(year)];
  }
  
  fprintf(stdout, "%s %02d, %04d %02d:%02d:%02d.%06ld\n", month, monthDay, year, hours, min, sec, now.tv_usec);
#endif /* DISABLE */
  return 0;
}

/*
 * cmdEcho :: [String] -> String
 *
 * Echo the input to the output, whitespace is collapsed.
 */
int cmdEcho(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (i > 1) fputc(' ', stdout);
    fputs(argv[i], stdout);
  }
  fputc('\n', stdout);

  return 0;
}

/*
 * cmdExit :: ()
 *
 * Exit the shell immediately
 */
int cmdExit(int argc, char *argv[]) {
  UNUSED(argc);
  UNUSED(argv);
  exit(0);
}

/*
 * cmdHelp :: String
 *
 * Displays a help message.
 */
int cmdHelp(int argc, char *argv[]) {
  UNUSED(argv);
  if (argc > 1)
	fputs("usage: help\n\n", stdout);
  
  fputs("\n", stdout);
  fputs("Shell Help:\n", stdout);
  fputs("\n", stdout);
  fputs("Commands:\n", stdout);
  fputs("\n", stdout);
  fputs("date              Prints the current date\n", stdout);
  fputs("echo [ARG...]     Echo arguments to output\n", stdout);
  fputs("exit              Exit the shell\n", stdout);
  fputs("perror            Print the exit status of the last command\n", stdout);
  fputs("help              Display this help\n", stdout);
  fputs("memorymap         Dumps the memory allocator state\n", stdout);
  fputs("set NAME = VALUE  Set an environment variable\n", stdout);
  fputs("unset NAME        Unsets environment variable\n", stdout);
  fputs("\n", stdout);
  fputs("Report bugs to <christopherheller@fas.harvard.edu>\n", stdout);
  fputs("\n", stdout);
  return 0;
}

/*
 * cmdMemmap :: String
 *
 * Dumps the current memory map
 */
void kmallocDump(FILE *out);
int cmdMemmap(int argc, char *argv[]) {
  UNUSED(argv);
  if (argc > 1)
	fputs("usage: memorymap\n\n", stdout);
  
  kmallocDump(stdout);
  return 0;
}
