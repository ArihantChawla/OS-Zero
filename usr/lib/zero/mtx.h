#ifndef __ZERO_MTX_H__
#define __ZERO_MTX_H__

#if !defined(ZERONEWMTX)
#define ZERONEWMTX 0
#endif

/*
 * Special thanks to Matthew 'kinetik' Gregan for help with the mutex code.
 * :)
 */

typedef volatile long zeromtx;

#if (__KERNEL__)
#undef PTHREAD
#define PTHREAD        0
#endif
#define ZEROMTXINITVAL 0L
#define ZEROMTXLKVAL   1L
#if (ZERONEWMTX)
#define ZEROMTXCONTVAL 2L
#endif

#include <zero/asm.h>
//#if (__KERNEL__) && (__MTKERNEL__)
#if (__KERNEL__)
#include <kern/proc/sched.h>
#elif (PTHREAD)
/* on some Linux setups, the pthread library declares no prototype */
extern int pthread_yield(void);
#endif
#if defined(__linux__) && !(__KERNEL__)
#include <sched.h>
#endif

#define mtxinit(lp) (*(lp) = ZEROMTXINITVAL)

/*
 * try to acquire mutex lock
 * - return non-zero on success, zero if already locked
 */
#define mtxtrylk(lp) mtxtrylk2((volatile long *)(lp), ZEROMTXLKVAL)
#define mtxlk(lp) mtxlk2((volatile long *)(lp), ZEROMTXLKVAL)
#define mtxunlk(lp) mtxunlk2((volatile long *)(lp), ZEROMTXLKVAL)

static __inline__ long
mtxtrylk2(volatile long *lp, long val)
{
    volatile long res;
    long          ret;

    res = m_cmpswap(lp, ZEROMTXINITVAL, val);
    ret = !res;

    return ret;
}

static __inline__ void
mtxlk2(volatile long *lp, long val)
{
    volatile long res = val;
    
    do {
        res = m_cmpswap(lp, ZEROMTXINITVAL, val);
        if (res) {
#if defined(__linux__) && !(__KERNEL__)
            sched_yield();
#elif (__KERNEL__)
            schedyield();
#elif (PTHREAD)
            pthread_yield();
#endif
        }
        /* TODO: should I activate m_waitint() here? :) */
    } while (res);

    return;
}

#if (ZERONEWMTX)
static __inline__ void
mtxunlk2(volatile long *lp, long val)
{
    volatile long res;
    
    res = m_cmpswap(lp, ZEROMTXLKVAL, ZEROMTXINITVAL);
    m_membar();

    return;
}
#else
static __inline__ void
mtxunlk2(volatile long *lp, long val)
{
    (void)val;
    *lp = ZEROMTXINITVAL;
    m_membar();

    return;
}
#endif

#endif /* __ZERO_MTX_H__ */

