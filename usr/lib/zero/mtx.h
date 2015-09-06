#ifndef __ZERO_MTX_H__
#define __ZERO_MTX_H__

#include <stddef.h>
#include <zero/param.h>
#include <zero/asm.h>

/*
 * Special thanks to Matthew 'kinetik' Gregan for help with the mutex code.
 * :)
 */

#if defined(__KERNEL__)
#undef PTHREAD
#endif

#include <zero/asm.h>
//#if defined(__KERNEL__) && (__MTKERNEL__)
#if defined(__KERNEL__)
#include <kern/proc/sched.h>
#elif defined(PTHREAD)
/* on some Linux setups, the pthread library declares no prototype */
extern int pthread_yield(void);
#endif
#if defined(__linux__) && !defined(__KERNEL__)
#include <sched.h>
#endif

#if defined(PTHREAD) && !defined(__KERNEL__)
#include <pthread.h>
#endif

#if defined(__KERNEL__) || (defined(ZEROMTX) && (!defined(PTHREAD)))
typedef volatile long   zeromtx;
#elif defined(PTHREAD)
typedef pthread_mutex_t zeromtx;
#endif

#if defined(__KERNEL__) || (defined(ZEROMTX) && (!defined(PTHREAD)))

#include <zero/thr.h>

#define MTXINITVAL 0
#define MTXLKVAL   1
#if defined(ZERONEWMTX)
#define MTXCONTVAL 2
#endif

#define mtxinit(lp) (*(lp) = MTXINITVAL)
#define mtxfree(lp) /* no-op */

/*
 * try to acquire mutex lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ long
mtxtrylk(volatile long *lp)
{
    volatile long res;
    long          ret;

    res = m_cmpswap(lp, MTXINITVAL, MTXLKVAL);
    ret = !res;

    return ret;
}

/*
 * acquire mutex lock
 * - allow other threads to run when blocking
 */
static __inline__ void
mtxlk(volatile long *lp)
{
    volatile long res;
    
    do {
        res = m_cmpswap(lp, MTXINITVAL, MTXLKVAL);
        if (res) {
            thryield();
        }
    } while (res);

    return;
}

/*
 * unlock mutex
 * - must use full memory barrier to guarantee proper write-ordering
 */
static __inline__ void
mtxunlk(volatile long *lp)
{
    *lp = MTXINITVAL;
    m_membar();

    return;
}

#define zerotrylkmtx(mp) mtxtrylk(mp)
#define zerolkmtx(mp)    mtxlk(mp)
#define zerounlkmtx(mp)  mtxunlk(mp)

#elif defined(PTHREAD) /* !defined(ZEROMTX) */

#define MTXINITVAL PTHREAD_MUTEX_INITIALIZER
typedef pthread_mutex_t zeromtx;

#define zerotrylkmtx(mp) pthread_mutex_trylock(mp)
#define zerolkmtx(mp)    pthread_mutex_lock(mp)
#define zerounlkmtx(mp)  pthread_mutex_unlock(mp)

#endif

#if defined(__KERNEL__) || (defined(ZEROMTX) && !defined(PTHREAD))

/* initializer for non-dynamic attributes */
#define ZEROMTXATR_DEFVAL     { 0L }
/* flags for attribute flg-field */
#define ZEROMTX_RECURSIVE     (1L << 0)
#define ZEROMTX_DETACHED      (1L << 1)
/* private flg-bits */
#define __ZEROMTXATR_DYNAMIC  (1L << 30)
#define __ZEROMTXATR_INIT     (1L << 31)
/* error codes */
#define ZEROMTXATR_NOTDYNAMIC 1
typedef struct {
    long flg;           // feature flag-bits
} zeromtxatr;

/* initializer for non-dynamic mutexes */
#define ZEROMTX_INITVAL { MTXINITVAL, ZEROMTXFREE, 0, 0, ZEROMTXATRDEFVAL }
/* thr for unlocked mutexes */
#define ZEROMTX_FREE    0
typedef struct {
    volatile long lk;
    volatile long val;  // owner for recursive mutexes, 0 if unlocked
    volatile long cnt;  // access counter
    volatile long rec;  // recursion depth
    zeromtxatr    atr;
} zeromtxrec;

#elif defined(PTHREAD)

#include <stddef.h>

#define MTXINITVAL PTHREAD_MUTEX_INITIALIZER

typedef pthread_mutexattr_t zeromtxatr;
typedef pthread_mutex_t     zeromtx;

#define mtxinit(mp)  pthread_mutex_init(mp, NULL)
#define mtxfree(mp)  pthread_mutex_destroy(mp)
#define mtxtrylk(mp) pthread_mutex_trylock(mp)
#define mtxlk(mp)    pthread_mutex_lock(mp)
#define mtxunlk(mp)  pthread_mutex_unlock(mp)

#endif

#endif /* __ZERO_MTX_H__ */

