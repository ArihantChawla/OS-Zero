#ifndef __TIME_H__
#define __TIME_H__

#include <features.h>
//#include <stddef.h>
#include <sys/types.h>
#include <unistd.h>
#include <zero/trix.h>
#if (_ZERO_SOURCE)
#include <kern/conf.h>
#endif
#if (_XOPEN_SOURCE) || (USESVID)
extern int   daylight;
extern long  timezone;
#endif
#if (_POSIX_SOURCE)
extern char *tzname[2];
#endif
extern char *__tzname[2];
extern long  __timezone;
extern int   _daylight;

#if (_ZERO_SOURCE)
//#include <kern/proc/thr.h>
#define CLOCKS_PER_SEC HZ
#else
#error CLOCKS_PER_SEC undefined
#endif
#if !(USEXOPEN2K)
#define CLK_TCK        CLOCKS_PER_SEC
#endif

#if (_POSIX_SOURCE) || (_XOPEN_SOURCE)
struct tm {
    int         tm_sec;		    // seconds [0, 60]
    int         tm_min;         // minutes [0, 59]
    int         tm_hour;        // hour [0, 23]
    int         tm_mday;        // day of month [1, 31]
    int         tm_mon;         // month of year [0, 11]
    int         tm_year;        // years since 1900
    int         tm_wday;        // day of week [0, 6] (sunday == 0)
    int         tm_yday;        // day of year [0, 365]
    int         tm_isdst;       // daylight savings flag (-1/0/1)
#if (_BSD_SOURCE)
    long        tm_gmtoff;		// seconds east (forward) of UTC/GMT
	const char *tm_zone;		// timezone abbreviation
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

extern clock_t clock(void);
extern time_t  time(time_t *tmr);
extern double  difftime(time_t time1, time_t time2);
extern time_t  mktime(struct tm *tm);
extern size_t  strftime(char *__restrict buf, size_t len,
                       const char *__restrict fmt,
                       const struct tm *__restrict tm);
#if (_XOPEN_SOURCE)
extern char *strptime(const char *__restrict str, const char *__restrict fmt,
                      struct tm *tm);
#endif
#if (_GNU_SOURCE) && 0 /* TODO: <xlocale.h> */
#include <xlocale.h>
extern size_t strftime_1(char *__restrict buf, size_t len,
                         const char *__restrict fmt,
                         const struct tm *__restrict tm, locale_t loc);
extern char   strptime_1(const char *__restrict str, const char *__restrict fmt,
                         struct tm *tp, locale_t loc);
#endif
extern struct tm *gmtime(const time_t *tmr);
extern struct tm *localtime(const time_t *tmr);
#if (_POSIX_SOURCE)
extern struct tm *gmtime_r(const time_t *__restrict tmr,
                           struct tm *__restrict tm);
extern struct tm *localtime_r(const time_t *__restrict tmr,
                              struct tm *__restrict tm);
#endif
extern char *asctime(const struct tm *tm);
extern char *ctime(const time_t *tmr);
#if (_POSIX_SOURCE)
extern char *asctime_r(const struct tm *tm,
                       char *__restrict buf);
extern char *ctime_r(const time_t *tmr,
                     char **__restrict buf);
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
#if 0 /* TODO: struct sigevent */
extern int    timer_create(clockid_t clk, struct sigevent *__restrict sigev,
                           timer_t *__restrict tmr);
#endif
extern int    timer_delete(timer_t tmr);
extern int    timer_settime(timer_t tmr, int flg,
                            const struct itimerspec *__restrict val,
                            struct itimerspec *__restrict *save);
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
#if (_GNU_SOURCE)
extern struct tm *getdate_r(const char *__restrict str,
                            struct tm *__restrict res);
#endif

#endif /* __TIME_H__ */

