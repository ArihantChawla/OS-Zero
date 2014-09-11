#ifndef __ZERO_MTX_H__
#define __ZERO_MTX_H__

/*
 * Special thanks to Matthew 'kinetik' Gregan for help with the mutex code.
 * :)
 */

#if (__KERNEL__)
#undef PTHREAD
#define PTHREAD    0
#endif
#define ZEROMTXINITVAL (0L)
#define ZEROMTXLKVAL   (1L)

#include <zero/asm.h>
#if (__KERNEL__) && (__MTKERNEL__)
#include <kern/proc/sched.h>
#elif (PTHREAD)
/* on some Linux setups, the pthread library declares no prototype */
extern int pthread_yield(void);
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

    return (res == ZEROMTXINITVAL);
}
#define mtxtrylk(lp) mtxtrylk2((volatile long *)(lp), ZEROMTXLKVAL)

static __inline__ void
mtxlk2(volatile long *lp, long val)
{
    volatile long res = val;
    
    do {
        res = m_cmpswap(lp, ZEROMTXINITVAL, val);
        if (res != ZEROMTXINITVAL) {
#if (__KERNEL__)
            schedpickthr();
#elif (PTHREAD)
            pthread_yield();
#endif
        }
    } while (res != ZEROMTXINITVAL);

    return;
}
#define mtxlk(lp) mtxlk2((volatile long *)(lp), ZEROMTXLKVAL)

static __inline__ void
mtxunlk2(volatile long *lp, long val)
{
    m_membar();
    *lp = ZEROMTXINITVAL;

    return;
}
#define mtxunlk(lp) mtxunlk2((volatile long *)(lp), ZEROMTXLKVAL)

#endif /* __ZERO_MTX_H__ */

