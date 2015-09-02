#ifndef __ZERO_SPIN_H__
#define __ZERO_SPIN_H__

#include <zero/asm.h>

typedef volatile long zerospin;

#define ZEROSPININITVAL 0L

#define spininit(sp) (*(sp) = ZEROSPININITVAL)

/*
 * try to acquire spin-lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ long
spintrylk(volatile long *sp, long val)
{
    volatile long res;
    long          ret;

    res = m_cmpswap(sp, ZEROSPININITVAL, val);
    ret = !res;
    
    return ret;
}

/*
 * lock spin-lock
 */
static __inline__ void
spinlk(volatile long *sp, long val)
{
    volatile long res = val;

    do {
        res = m_cmpswap(sp, ZEROSPININITVAL, val);
    } while (res);

    return;
}

/*
 * release spin-lock
 */
static __inline__ void
spinunlk(volatile long *sp, long val)
{
    *sp = ZEROSPININITVAL;
    m_membar();
}

#endif /* __ZERO_SPIN_H__ */

