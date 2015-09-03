#ifndef __ZERO_SPINWT_H__
#define __ZERO_SPINWT_H__

/*
 * NOTES
 * -----
 * - initialise spin-wait locks with spininit() from <zero/spin.h>
 */

#include <zero/asm.h>
#include <zero/spin.h>

/*
 * try to acquire spin-wait lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ void
spinwttrylk(volatile long *sp, long val, long niter)
{
    volatile long res = val;
    long          ret;

    if (niter) {
        do {
            res = m_cmpswap(sp, ZEROSPININITVAL, val);
        } while ((--niter) && (res));
    } else {
        res = m_cmpswap(sp, ZEROSPININITVAL, val);
    }
    ret = !res;
    
    return ret;
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
            res = m_cmpswap(sp, ZEROSPININITVAL, val);
        } while ((--niter) && (res));
    } else {
        res = m_cmpswap(sp, ZEROSPININITVAL, val);
    }
    if (res) {
        mtxlk(sp, val);
    }

    return;
}

/*
 * release spin-wait lock
 */
static __inline__ void
spinwtunlk(volatile long *sp, long val, long niter)
{
    *sp = ZEROSPININITVAL;
    m_membar();
}

#endif /* __ZERO_SPINWT_H__ */

