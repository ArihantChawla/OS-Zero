#ifndef __TIME_H__
#define __TIME_H__

#include <features.h>
#include <stddef.h>
#include <sys/types.h>
#if (_ZERO_SOURCE)
#include <kern/conf.h>
#endif
#include <signal.h>
#if (_XOPEN_SOURCE) || (USESVID)
extern int   daylight;
extern long  timezone;
#endif
#if (_POSIX_SOURCE)
extern char *tzname[2];
#endif
#if 0
extern char *__tzname[2];
extern long  __timezone;
extern int   __daylight;
#endif

//#include <bits/signal.h>

#if !defined(NULL)
#define NULL               ((void *)0L)
#endif

#if (_ZERO_SOURCE)
//#include <kern/proc/thr.h>
#define CLOCKS_PER_SEC           1000000000     // for nanosecond resolution
#else
#error CLOCKS_PER_SEC undefined
#endif
#if !(USEXOPEN2K)
#define CLK_TCK                  CLOCKS_PER_SEC
#endif

#define CLOCK_MONOTONIC          0      // monotonic time
#define CLOCK_PROCESS_CPUTIME_ID 1      // pre-process CPU-time clock
#define CLOCK_REALTIME           2      // system-wide wall-clock time
#define CLOCK_THREAD_CPUTIME_ID  3      // thread-specific CPU-time clock
#if defined(__linux__)
#define CLOCK_BOOTTIME           4
#define CLOCK_MONOTONIC_RAW      5
#define CLOCK_MONOTONIC_COARSE   6      // faster, less precise monotonic time
#define CLOCK_REALTIME_COARSE    7      // faster, less precise wall-clock
#endif

#define TIMER_ABSTIME            (1 << 0)

#if (_POSIX_SOURCE) || (_XOPEN_SOURCE)
struct tm {
    /* FIXME: at lest tm_sec _really_ needs to be a 64-bit type :) */
    int         tm_sec;		// seconds [0, 60]
    int         tm_min;         // minutes [0, 59]
    int         tm_hour;        // hour [0, 23]
    int         tm_mday;        // day of month [1, 31]
    int         tm_mon;         // month of year [0, 11]
    int         tm_year;        // years since 1900
    int         tm_wday;        // day of week [0, 6] (sunday == 0)
    int         tm_yday;        // day of year [0, 365]
    int         tm_isdst;       // daylight savings flag (-1/0/1)
    int         _pad;
#if (_BSD_SOURCE)
    long        tm_gmtoff;	// seconds east (forward) of UTC/GMT
    const char *tm_zone;	// timezone abbreviation
#else
    long        __tm_gmtoff;    // seconds forward of UTC/GMT§
    const char *__tm_zone;
#endif
};
#endif

#if (USEPOSIX199309)
struct timespec {
    time_t tv_sec;
    long   tv_nsec;
};
#endif

struct itimerspec {
    struct timespec it_interval;        // timer period
    struct timespec it_value;           // timer expiration
};

#if !defined(__KERNEL__)

extern clock_t clock(void);
extern time_t  time(time_t *tmr);
extern double  difftime(time_t time1, time_t time2);
extern time_t  mktime(struct tm *tm);
extern size_t  strftime(char *restrict buf, size_t len,
                       const char *restrict fmt,
                       const struct tm *restrict tm);
#if (_XOPEN_SOURCE)
extern char *strptime(const char *restrict str, const char *restrict fmt,
                      struct tm *tm);
#endif
#if defined(_GNU_SOURCE) && 0 /* TODO: <xlocale.h> */
#include <xlocale.h>
extern size_t strftime_1(char *restrict buf, size_t len,
                         const char *restrict fmt,
                         const struct tm *restrict tm, locale_t loc);
extern char   strptime_1(const char *restrict str, const char *restrict fmt,
                         struct tm *tp, locale_t loc);
#endif
extern struct tm *gmtime(const time_t *tmr);
extern struct tm *localtime(const time_t *tmr);
#if (_POSIX_SOURCE)
extern struct tm *gmtime_r(const time_t *restrict tmr,
                           struct tm *restrict tm);
extern struct tm *localtime_r(const time_t *restrict tmr,
                              struct tm *restrict tm);
#endif
extern char *asctime(const struct tm *tm);
extern char *ctime(const time_t *tmr);
#if (_POSIX_SOURCE)
extern char *asctime_r(const struct tm *tm,
                       char *restrict buf);
extern char *ctime_r(const time_t *tmr,
                     char **restrict buf);
#endif
#if (_POSIX_SOURCE)
extern void tzset(void);
#endif
#if (USESVID)
extern int  stime(const time_t when);
#endif
#define     _isleap(year) leapyear(year)

extern time_t timegm(struct tm *tm);
extern time_t timelocal(struct tm *tm);
extern int    dysize(int year);

#if (USEPOSIX199309)
extern int    nanosleep(const struct timespec *req, struct timespec *left);
extern int    clock_getres(clockid_t clk, struct timespec *res);
extern int    clock_gettime(clockid_t clk, struct timespec *ts);
extern int    clock_settime(clockid_t clk, const struct timespec *ts);
#if (USEXOPEN2K)
extern int    clock_nanosleep(clockid_t clk, int flg,
                              const struct timespec *req,
                              struct timespec *left);
extern int    clock_getcpuclockid(pid_t pid, clockid_t *clk);
#endif
struct sigevent;
extern int    timer_create(clockid_t clk, struct sigevent *restrict sigev,
                           timer_t *restrict tmr);
extern int    timer_delete(timer_t tmr);
extern int    timer_settime(timer_t tmr, int flg,
                            const struct itimerspec *restrict val,
                            struct itimerspec *restrict *save);
extern int    timer_gettime(timer_t tmr, struct itimerspec *val);
extern int    timer_getoverrun(timer_t tmr);
#endif
#if (USEXOPENEXT)
#define GETDATE_DATEMASK_INVAL 1        // DATEMSK is null or undefined
#define GETDATE_CANNOT_READ    2        // template file not readable
#define GETDATE_STAT_FAIL      3        // failed to get file status information
#define GETDATE_NOT_REG_FILE   4        // template is not regular file
#define GETDATE_READ_ERROR     5        // error reading template file
#define GETDATE_NOMEM          6        // memory allocation failure
#define GETDATE_NO_TEMPL       7        // no template line matches
#define GETDATE_INPUT_INVAL    8        // invalid input specification
extern int getdate_err;

extern struct tm *getdate(const char *str);
#endif
#if defined(_GNU_SOURCE)
extern struct tm *getdate_r(const char *restrict str,
                            struct tm *restrict res);
#endif

#endif /* !defined(__KERNEL__) */

#endif /* __TIME_H__ */

