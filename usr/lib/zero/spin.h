#ifndef __ZERO_SPIN_H__
#define __ZERO_SPIN_H__

#ifndef SPINOWNER
#define SPINOWNER 0
#endif

#include <zero/asm.h>

#define SPININITVAL 0L

#define spininit(sp) (*(sp) = SPININITVAL)

/*
 * try to acquire spin-lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ long
spintrylk(volatile long *sp, long val)
{
    volatile long res;

    res = m_cmpswap(sp, SPININITVAL, val);

    return (res == SPININITVAL);
}

/*
 * lock spin-lock
 */
static __inline__ void
spinlk(volatile long *sp, long val)
{
    while (m_cmpswap(sp, SPININITVAL, val) != SPININITVAL) {
        ;
    }

    return;
}

/*
 * release spin-lock
 */
static __inline__ void
spinunlk(volatile long *sp, long val)
{
#if (SPINOWNER)
    while (m_cmpswap(sp, val, SPININITVAL)) {
        ;
    }
#else /* !SPINOWNER */
    m_membar();
    *sp = SPININITVAL;
#endif
}

#endif /* __ZERO_SPIN_H__ */

