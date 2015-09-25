#ifndef __ZERO_TIME_H__
#define __ZERO_TIME_H__

#include <features.h>
#include <time.h>

#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 199309L)
static __inline__ time_t
timegetstamp(void)
{
    struct timespec ts;
    time_t          tm;

#if defined(CLOCK_MONOTONIC)
    clock_gettime(CLOCK_MONOTONIC, &ts);
#else
    clock_gettime(CLOCK_REALTIME, &ts);
#endif
    tm = ts.tv_sec * 1000000000 + ts.tv_nsec;

    return tm;
}
#endif /* _POSIX_C_SOURCE */

#define tsdiffnsec(ts1, ts2)                                            \
    (((ts2)->tv_sec - (ts1)->tv_sec) * 1000000000                       \
     + (time_t)(ts2)->tv_nsec - (time_t)(ts1)->tv_nsec)

#endif /* __ZERO_TIME_H__ */

