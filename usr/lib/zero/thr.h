#ifndef __ZERO_THR_H__
#define __ZERO_THR_H__

#include <stdint.h>
#if defined(PTHREAD) && !defined(ZEROTHR)
#include <pthread.h>
#endif
#include <zero/mtx.h>

#if defined(PTHREAD)
#define thrid() ((uintptr_t)pthread_self())
#endif

#if defined(_WIN64) || defined(_WIN32)
#define thryield() kYieldProcessor()
#elif defined(__linux__) && !defined(__KERNEL__)
#define thryield() sched_yield();
#elif defined(__KERNEL__)
#define thryield() schedyield();
#elif defined(PTHREAD) && !defined(ZEROTHR)
#define thryield() pthread_yield();
#elif defined(ZEROTHR)
#define thryield() /* FIXME */
#endif

#if defined(PTHREAD) && !defined(ZEROTHR)

typedef pthread_t      zerothr;
typedef pthread_attr_t zerothratr;

#elif defined(ZEROTHR)

#include <stddef.h>
#include <sched.h>

#define ZEROTHRATR_INIT         (1 << 0)        // attributes initialised
#define ZEROTHRATR_DETACHED     (1 << 1)        // detach thread
#define ZEROTHRATR_CPUSET       (1 << 2)        // affinity configuration
#define ZEROTHRATR_INHERITSCHED (1 << 3)        // inherit scheduler parameters
#define ZEROTHRATR_SCHED_PARAM  (1 << 4)        // set scheduler parameters
typedef struct {
    long                flg;
    void               *ncpu;
    void               *cpuset;
    struct sched_param  schedparm;
    void               *stkadr;
    size_t              stksize;
    size_t              guardsize;
} zerothratr;

#endif

#define ZEROTHR_NOID   (~(uintptr_t)0)
#define ZEROTHR_ASLEEP 1
#define ZEROTHR_AWAKE  0
typedef struct __zerothr {
    uintptr_t         id;
    long              sleep;
    zerothratr       *atr;
    struct __zerothr *prev;
    struct __zerothr *next;
} zerothr;

typedef struct {
    zeromtx  lk;
    zerothr *head;
    zerothr *tail;
} zerothrqueue;

extern zerothrqueue thrsleepqueue;

#define thrsleep()   thrsleep1(&thrsleepqueue)
#define thrwake()    thrwake1(&thrsleepqueue)
#define thrwakeall() thrwakeall1(&thrsleepqueue)

#endif /* __ZERO_THR_H__ */

