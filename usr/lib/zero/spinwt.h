#ifndef __ZERO_SPINWT_H__
#define __ZERO_SPINWT_H__

#include <zero/asm.h>
#include <zero/spin.h>

/*
 * try to acquire spin-wait lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ void
spinwttrylk(volatile long *sp, long val)
{
    volatile long res = val;

    if (niter) {
        do {
            res = m_cmpswap(sp, SPININITVAL, val);
        } while ((--niter) && (res != SPININITVAL));
    } else {
        res = m_cmpswap(sp, SPININITVAL, val);
    }

    return (res == SPININITVAL);
}

/*
 * lock spin-wait lock
 */
static __inline__ void
spinwtlk(volatile long *sp, long val, long niter)
{
    volatile long res = val;

    if (niter) {
        do {
            res = m_cmpswap(sp, SPININITVAL, val);
        } while ((--niter) && (res != SPININITVAL));
    } else {
        res = m_cmpswap(sp, SPININITVAL, val);
    }
    if (res != SPININITVAL) {
        mtxlk(sp, val);
    }

    return;
}

/*
 * release mutex lock
 */
static __inline__ void
spinwtunlk(volatile long *sp, long val, long niter)
{
#if (SPINOWNER)
    volatile long res = val;
#endif

#if (SPINOWNER)
    if (niter) {
        do {
            res = m_cmpswap(sp, SPININITVAL, val);
        } while ((--niter) && (res != SPININITVAL));
    } else {
        res = m_cmpswap(sp, SPININITVAL, val);
    }
    if (res != SPININITVAL) {
        mtxunlk(sp, val);
    }
#else /* !SPINOWNER */
    m_membar();
    *sp = SPININITVAL;
#endif
}

#endif /* __ZERO_SPIN_H__ */

