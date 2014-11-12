#ifndef __ZERO_MTX_H__
#define __ZERO_MTX_H__

/*
 * Special thanks to Matthew 'kinetik' Gregan for help with the mutex code.
 * :)
 */

#if (__KERNEL__)
#undef PTHREAD
#define PTHREAD        0
#endif
#define ZEROMTXINITVAL 0L
#define ZEROMTXLKVAL   1L
#define ZEROMTXCONTVAL 2L

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

    return (res == ZEROMTXINITVAL) ? ZEROMTXLKVAL : 0;
}
#define mtxtrylk(lp) mtxtrylk2((volatile long *)(lp), ZEROMTXLKVAL)

static __inline__ long
mtxlk2(volatile long *lp, long val)
{
    volatile long res = val;
    
    do {
        res = m_cmpswap(lp, ZEROMTXINITVAL, val);
        if (res == ZEROMTXINITVAL) { 

            return ZEROMTXLKVAL;
        }
        if (res == ZEROMTXLKVAL) {
#if (__KERNEL__)
            schedpickthr();
#elif (PTHREAD)
            pthread_yield();
#endif
        } else {
            res = m_cmpswap(lp, ZEROMTXCONTVAL, ZEROMTXLKVAL);
            if (res == ZEROMTXCONTVAL) {

                return res;
            }
        }
    } while (res != ZEROMTXINITVAL);

    return res;
}
#define mtxlk(lp) mtxlk2((volatile long *)(lp), ZEROMTXLKVAL)

static __inline__ void
mtxunlk2(volatile long *lp, long val)
{
    volatile long unlk = m_cmpswap(lp, ZEROMTXINITVAL, ZEROMTXLKVAL);
    volatile long cont = m_cmpswap()
    membar();
    if (mtx != ZEROMTXLKVAL)

        retu
//    m_membar();

    *lp = val;

    return val;
}
#define mtxunlk(lp) mtxunlk2((volatile long *)(lp), ZEROMTXINITVAL)

#endif /* __ZERO_MTX_H__ */

