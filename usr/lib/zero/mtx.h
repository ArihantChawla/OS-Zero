#ifndef __ZERO_MTX_H__
#define __ZERO_MTX_H__

/*
 * Special thanks to Matthew 'kinetik' Gregan for help with the mutex code.
 * :)
 */

#include <zero/asm.h>
#if (__KERNEL__) && (__MTKERNEL__)
#include <kern/proc/sched.h>
#else
/* on some Linux setups, the pthread library declares no prototype */
extern int pthread_yield(void);
#endif

#define MTXINITVAL 0L
#define MTXLKVAL   1L

#define mtxinit(lp) (*(lp) = MTXINITVAL)

/*
 * try to acquire mutex lock
 * - return non-zero on success, zero if already locked
 */
#define mtxtrylk(lp) mtxtrylk2(lp, MTXLKVAL)

static __inline__ long
mtxtrylk2(volatile long *lp, long val)
{
    volatile long res;

    res = m_cmpswap(lp, MTXINITVAL, val);

    return (res == MTXINITVAL);
}

#define mtxlk(lp) mtxlk2(lp, MTXLKVAL)
static __inline__ void
mtxlk2(volatile long *lp, long val)
{
    volatile long res = val;

    do {
        res = m_cmpswap(lp, MTXINITVAL, val);
        if (res != MTXINITVAL) {
#if (__KERNEL__)
            schedyield();
#else
            pthread_yield();
#endif
        }
    } while (res != MTXINITVAL);

    return;
}

#define mtxunlk(lp) mtxunlk2(lp, MTXLKVAL)
static __inline__ void
mtxunlk2(volatile long *lp, long val)
{
    m_membar();
    *lp = MTXINITVAL;

    return;
}

#endif /* __ZERO_MTX_H__ */

