#ifndef __ZERO_THR_H__
#define __ZERO_THR_H__

#include <stddef.h>
#include <stdint.h>

#if defined(PTHREAD) && !defined(__KERNEL__)
#define thrid() ((uintptr_t)pthread_self())
#endif

#if defined(_WIN64) || defined(_WIN32)
#define thryield() kYieldProcessor()
#elif defined(__linux__) && !defined(__KERNEL__)
#define thryield() sched_yield();
#elif defined(__KERNEL__)
extern void schedyield(void);
#define thryield() schedyield();
#elif defined(PTHREAD) || defined(ZEROPTHREAD)
#define thryield() pthread_yield();
//#elif defined(ZEROTHR)
//#define thryield() /* FIXME */
#endif

#include <zero/mtx.h>

#if !defined(__KERNEL__)

#if defined(ZEROTHR)

#include <sched.h>

typedef uintptr_t zerothrid;

#define ZEROTHRATR_INIT          (1 << 0)       // attributes initialised
#define ZEROTHRATR_DETACHED      (1 << 1)       // detach thread
#define ZEROTHRATR_INHERITSCHED  (1 << 2)       // inherit scheduler parameters
#define ZEROTHRATR_EXPLICITSCHED (1 << 3)       // specify scheduler parameters
#define ZEROTHRATR_SCHED_PARAM   (1 << 4)       // scheduler parameters
#define ZEROTHRATR_SCHEDPOLICY   (1 << 5)       // scheduler policy
#define ZEROTHRATR_SCOPE         (1 << 6)       // scheduling scope
#define ZEROTHRATR_STKATR        (1 << 7)       // stack address and size
#define ZEROTHRATR_GUARDSIZE     (1 << 8)       // stack guard size
#define ZEROTHRATR_AFFINITY      (1 << 9)       // affinity configuration
typedef struct __zerothratr {
    long                flg;
    void               *stkadr;
    size_t              stksize;
    size_t              guardsize;
#if defined(_GNU_SOURCE)
    size_t              ncpu;
    void               *cpuset;
#endif
    struct sched_param  schedparm;
    uint8_t             _res[4 * CLSIZE - sizeof(struct sched_param)
#if defined(_GNU_SOURCE)
                             - sizeof(size_t) - sizeof(void *)
#endif
                             - 2 * sizeof(size_t) - sizeof(void *)
                             - sizeof(long)];
} zerothratr;

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
    volatile long  lk;
    zerothr       *head;
    zerothr       *tail;
} zerothrqueue;

extern void thrwait1(zerothrqueue *queue);
extern long thrsleep2(zerothrqueue *queue, const struct timespec *absts);
extern void thrwake1(zerothrqueue *queue);
extern void thrwakeall1(zerothrqueue *queue);

#define thrwait()    thrwait1(NULL)
#define thrwake()    thrwake1(NULL)
#define thrwakeall() thrwakeall1(NULL)

#endif /* defined(ZEROTHR) */

#endif /* !defined(__KERNEL__) */

#endif /* __ZERO_THR_H__ */

