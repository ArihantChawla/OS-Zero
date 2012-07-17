#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <features.h>
#include <sys/select.h>

#if (_ZERO_SOURCE)
#include <kern/thr.h>
#define CLOCKS_PER_SEC HZ
#else
#error CLOCKS_PER_SEC undefined
#endif
#ifndef CLK_TCK
#define CLK_TCK        CLOCKS_PER_SEC
#endif

#if (_XOPEN_SOURCE || _POSIX_SOURCE || _UNIX_SOURCE)
struct tm {
    long tm_sec;
    long tm_min;
    long tm_hour;
    long tm_mday;
    long tm_mon;
    long tm_year;
    long tm_wday;
    long tm_yday;
    long tm_isdst;
#if (_BSD_SOURCE)
    long tm_gmtoff;
#else
    long __tm_gmtoff;
    long __tm_zone;
#endif
};
#endif

#if (_POSIX_C_SOURCE >= 199309L)
struct itimerspec {
    struct timespec it_interval;
    struct timespec it_value;
};
#endif

#endif /* __SYS_TIME_H__ */

