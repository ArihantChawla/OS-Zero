#ifndef __ZERO_TIME_H__
#define __ZERO_TIME_H__

#define tsdiffnsec(ts1, ts2)                                            \
    (((ts2)->tv_sec - (ts1)->tv_sec) * 1000000000                       \
     + ((ts2)->tv_nsec - (ts1)->tv_nsec))

#endif /* __ZERO_TIME_H__ */

