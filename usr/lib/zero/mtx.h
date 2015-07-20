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
#define ZEROMTXINITVAL 0
#define ZEROMTXLKVAL   1

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
static __inline__ long
mtxtrylk2(volatile long *lp, long val)
{
    volatile long res;

    res = m_cmpswap(lp, ZEROMTXINITVAL, val);

#if (ZERONEWMTX)
    if (res == ZEROMTXINITVAL) {

        return ZEROMTXLKVAL;
    } else {

        return 0;
    }
#else
    return (res == ZEROMTXINITVAL);
#endif
}
#define mtxtrylk(lp) mtxtrylk2((volatile long *)(lp), ZEROMTXLKVAL)

static __inline__ long
mtxlk2(volatile long *lp, long val)
{
    volatile long res = val;
    
    do {
        res = m_cmpswap(lp, ZEROMTXINITVAL, val);
        if (res != ZEROMTXINITVAL) {
#if defined(__linux__) && !(__KERNEL__)
            sched_yield();
#elif (__KERNEL__)
            schedyield();
#elif (PTHREAD)
            pthread_yield();
#endif
        }
        /* TODO: should I activate m_waitint() here? :) */
    } while (res != ZEROMTXINITVAL);

    return ZEROMTXLKVAL;
}
#define mtxlk(lp) mtxlk2((volatile long *)(lp), ZEROMTXLKVAL)

#if (ZERONEWMTX)
static __inline__ long
mtxunlk2(volatile long *lp, long val)
{
    volatile long res;
    
    m_membar();
//    *lp = ZEROMTXINITVAL;
    res = m_cmpswap(lp, ZEROMTXLKVAL, ZEROMTXINITVAL);

    return (res == ZEROMTXLKVAL);
}
#else
static __inline__ void
mtxunlk2(volatile long *lp, long val)
{
    m_membar();
    *lp = ZEROMTXINITVAL;

    return;
}
#endif
#define mtxunlk(lp) mtxunlk2((volatile long *)(lp), ZEROMTXLKVAL)

#endif /* __ZERO_MTX_H__ */

