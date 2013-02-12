#ifndef __ZERO_MTX_H__
#define __ZERO_MTX_H__

/*
 * Special thanks to Matthew 'kinetik' Gregan for help with the mutex code.
 * :)
 */

//#define MTXOWNER 1
#ifndef MTXOWNER
#define MTXOWNER 0
#endif

#include <zero/asm.h>
#if (__KERNEL__) && (__MTKERNEL__)
#include <kern/sched.h>
#else
/* on some Linux setups, the pthread library declares no prototype */
extern int pthread_yield(void);
#endif

#define MTXINITVAL 0L

#define mtxinit(lp) (*(lp) = MTXINITVAL)

/*
 * try to acquire mutex lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ long
mtxtrylk(volatile long *lp, long val)
{
    long res;

    res = m_cmpswap(lp, MTXINITVAL, val);

    return (res == MTXINITVAL);
}

/*
 * lock mutex
 * - allow other threads to run while unsuccessful
 */
static __inline__ void
mtxlk(volatile long *lp, long val)
{
    while (m_cmpswap(lp, MTXINITVAL, val) != MTXINITVAL) {
#if (__KERNEL__)
        thryield();
#else
        pthread_yield();
#endif
    }

    return;
}

/*
 * release mutex lock
 */
static __inline__ void
mtxunlk(volatile long *lp, long val)
{
#if (MTXOWNER)
    while (m_cmpswap(lp, val, MTXINITVAL)) {
#if (__KERNEL__)
        thryield();
#else
        pthread_yield();
#endif
    }
#else /* !MTXOWNER */
    m_membar();
    *lp = MTXINITVAL;
#endif
}

#endif /* __ZERO_MTX_H__ */

