#ifndef _TIME_H
#define _TIME_H

#include <string.h>
#include <stdint.h>

typedef unsigned long time_t;
typedef uint64_t clock_t;
typedef uint64_t useconds_t;

struct tm {
    int tm_sec;         /* seconds */
    int tm_min;         /* minutes */
    int tm_hour;        /* hours */
    int tm_mday;        /* day of the month */
    int tm_mon;         /* month */
    int tm_year;        /* year */
    int tm_wday;        /* day of the week */
    int tm_yday;        /* day in the year */
    int tm_isdst;       /* daylight saving time */
};

#define LC_ALL		1
#define LC_COLLATE	2
#define LC_CTYPE	3
#define LC_MONETARY	4
#define LC_NUMERIC	5
#define LC_TIME		6

time_t time(time_t *timer);
clock_t clock();

char *asctime(const struct tm *tm);
char *ctime(const time_t *timep);

struct tm *gmtime(const time_t *timep);
struct tm *localtime(const time_t *timep);
double difftime(time_t time1, time_t time0);
time_t mktime(struct tm *tm);

size_t strftime(char *s, size_t max, const char *format, const struct tm *tm);

int usleep(useconds_t);

/* fuken posix */
#define CLOCKS_PER_SEC 1000000

#endif