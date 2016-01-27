#ifndef __SHARE_TIME_H__
#define __SHARE_TIME_H__

#if !defined(__time_types_defined)
typedef long            clock_t;
typedef long            clockid_t;
typedef long long       time_t;
typedef long            timer_t;
typedef unsigned long   useconds_t;
typedef long            suseconds_t;
#define __time_types_defined 1
#endif

#endif /* __SHARE_TIME_H__ */

