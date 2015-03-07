#ifndef __ZERO_PROF_H__
#define __ZERO_PROF_H__

#include <stdint.h>
#include <sys/time.h>

#if defined(__x86_64__) || defined(__amd64__) || defined(__i386__)
#include <zero/ia32/prof.h>
#elif defined(__arm__)
#include <zero/arm/prof.h>
#endif

#define tvcmp(tv1, tv2)                                                 \
    (((tv2)->tv_sec - (tv1)->tv_sec) * 1000000                          \
     + ((tv2)->tv_usec - (tv1)->tv_usec))
#define tvgt(tv1, tv2)                                                  \
    (((tv1)->tv_sec > (tv2)->tv_sec)                                    \
     || ((tv1)->tv_sec == (tv2)->tv_sec && (tv1)->tv_usec > (tv2)->tv_usec))

#define tvaddconst(tv, u)                                               \
  do {                                                                  \
      unsigned long _ms = 1000000;                                      \
                                                                        \
      (tv)->tv_sec += (u) / _ms;                                        \
      (tv)->tv_usec += (u) % _ms;                                       \
                                                                        \
      if ((tv)->tv_usec >= _ms) {                                       \
          (tv)->tv_sec++;                                               \
          (tv)->tv_usec -= _ms;                                         \
      } else if ((tv)->tv_usec < 0) {                                   \
          (tv)->tv_sec--;                                               \
          (tv)->tv_usec += _ms;                                         \
      }                                                                 \
  } while (FALSE)

#define PROFDECLCLK(id)                                                 \
    struct timeval __tv##id[2]
#if 0
#define profinitclk(id)                                                 \
    memset(&__tv##id, 0, sizeof(__tv##id))
#endif
#define profinitclk(id)                                                 \
    (__tv##id[0] = __tv##id[1] = { { 0, 0 }, { 0, 0 }}
    __
#define profstartclk(id)                                                \
    gettimeofday(&__tv##id[0], NULL)
#define profstopclk(id)                                                 \
    gettimeofday(&__tv##id[1], NULL)
#define profclkdiff(id)                                                 \
    tvcmp(&__tv##id[0], &_tv##id[1])

#endif /* __ZERO_PROF_H__ */

