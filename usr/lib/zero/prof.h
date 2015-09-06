#ifndef __ZERO_PROF_H__
#define __ZERO_PROF_H__

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>
#if defined(_MSC_VER)
#undef  __inline__
#define __inline__ inline
#include <time.h>
#endif
#include <sys/time.h>

#if defined(_MSC_VER) || defined(__x86_64__) || defined(__amd64__) || defined(__i386__)
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

#if defined(_MSC_VER)
#else
#define tpcmp(tp1, tp2)                                                 \
    (((tp2)->tv_sec - (tp1)->tv_sec) * 1000000                          \
     + ((tp2)->tv_usec - (tp1)->tv_usec))
#define tpgt(tp1, tp2)                                                  \
    (((tp1)->tv_sec > (tp2)->tv_sec)                                    \
     || ((tp1)->tv_sec == (tp2)->tv_sec && (tp1)->tv_usec > (tp2)->tv_usec))
#endif

#define tvaddconst(tv, u)                                               \
  do {                                                                  \
      unsigned long _us = 1000000;                                      \
                                                                        \
      (tv)->tv_sec += (u) / _us;                                        \
      (tv)->tv_usec += (u) % _us;                                       \
                                                                        \
      if ((tv)->tv_usec >= _us) {                                       \
          (tv)->tv_sec++;                                               \
          (tv)->tv_usec -= _us;                                         \
      } else if ((tv)->tv_usec < 0) {                                   \
          (tv)->tv_sec--;                                               \
          (tv)->tv_usec += _us;                                         \
      }                                                                 \
  } while (FALSE)

#define PROFDECLCLK(id)                                                 \
    struct timeval __tv##id[2]
#define profinitclk(id)                                                 \
    memset(&__tv##id, 0, sizeof(__tv##id))
#define profstartclk(id)                                                \
    gettimeofday(&__tv##id[0], NULL)
#define profstopclk(id)                                                 \
    gettimeofday(&__tv##id[1], NULL)
#define profclkdiff(id)                                                 \
    tvcmp(&__tv##id[0], &__tv##id[1])

#if defined(_MSC_VER) && defined(__cplusplus)
#define PROFDECLOS(id)                                                  \
    LARGE_INTEGER __li##id[2]
#define profstartos(id)                                                 \
    QueryPerformanceCounter(&__li##id[0]
#define profstopos(id)
    QueryPerformanceCounter(&__li##id[1])
#define profosdiff(id)                                                  \
    (__li##id[1] - __li##id[0])
#else
#define PROFDECLOS(id)                                                  \
    struct timespec __ts##id[2]
#define profstartos(id)                                                 \
    clock_gettime(CLOCK_REALTIME, &__ts##id[0])
#define profstopos(id)                                                  \
    clock_gettime(CLOCK_REALTIME, &__ts##id[1])
#define profosdiff(id)                                                  \
    tpcmp(&__ts##id[0], &__ts##id[1])
#endif

#if defined(__cplusplus)
}
#endif

#endif /* __ZERO_PROF_H__ */

