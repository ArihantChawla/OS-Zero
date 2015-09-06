#ifndef __ZERO_THR_H__
#define __ZERO_THR_H__

#define ZEROTHR 1

#include <stddef.h>
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

#if defined(PTHREAD)

typedef pthread_t      zerothrid;
typedef pthread_attr_t zerothratr;

#elif defined(ZEROTHR)

#include <sched.h>

typedef uintptr_t      zerothrid;

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

#define ZEROTHR_NOID   (~(zerothrid)0)
#define ZEROTHR_ASLEEP 1
#define ZEROTHR_AWAKE  0
typedef struct __zerothr {
    zerothrid         id;
    long              sleep;
    zerothratr       *atr;
    struct __zerothr *prev;
    struct __zerothr *next;
} zerothr;

#define ZEROTHRQUEUE_INITIALIZER { MTXINITVAL, NULL, NULL }
typedef struct {
    zeromtx  lk;
    zerothr *head;
    zerothr *tail;
} zerothrqueue;

extern void thrwait1(zerothrqueue *queue);
extern long thrsleep2(zerothrqueue *queue, const struct timespec *absts);
extern void thrwake1(zerothrqueue *queue);
extern void thrwakeall1(zerothrqueue *queue);

#define thrwait()    thrwait1(NULL)
#define thrwake()    thrwake1(NULL)
#define thrwakeall() thrwakeall1(NULL)

#endif /* __ZERO_THR_H__ */

