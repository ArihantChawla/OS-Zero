#ifndef __TIME_H__
#define __TIME_H__

#include <features.h>
#include <stddef.h>
#include <unistd.h>

extern int   daylight;
extern long  timezone;
extern char *tzname[];

#if (_ZERO_SOURCE)
#include <kern/proc/thr.h>
#define CLOCKS_PER_SEC HZ
#else
#error CLOCKS_PER_SEC undefined
#endif

struct tm {
    int tm_sec;         // seconds [0, 60]
    int tm_min;         // minutes [0, 59]
    int tm_hour;        // hour [0, 23]
    int tm_mday;        // day of month [1, 31]
    int tm_mon;         // month of year [0, 11]
    int tm_year;        // years since 1900
    int tm_wday;        // day of week [0, 6] (sunday == 0)
    int tm_yday;        // day of year [0, 365]
    int tm_isdst;       // daylight savings flag
#if (_BSD_SOURCE)
    long tm_gmtoff;
#else
    long __tm_gmtoff;
    long __tm_zone;
#endif
};

struct timespec {
    time_t tv_sec;
    long   tv_nsec;
};

struct itimerspec {
    struct timespec it_interval;        // timer period
    struct timespec it_value;           // timer expiration
};

#endif /* __TIME_H__ */

