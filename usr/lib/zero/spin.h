#ifndef __ZERO_SPIN_H__
#define __ZERO_SPIN_H__

#include <zero/asm.h>

typedef volatile long zerospin;

#define ZEROSPININITVAL 0L
#define ZEROSPINLKVAL   1L

#define spininit(sp) (*(sp) = ZEROSPININITVAL)

/*
 * try to acquire spin-lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ long
spintrylk(volatile long *sp)
{
    volatile long res;

    res = m_cmpswap(sp, ZEROSPININITVAL, ZEROSPINLKVAL);
    if (res == ZEROSPININITVAL) {
        res++;
    }
    
    return res;
}

/*
 * lock spin-lock
 */
static __inline__ long
spinlk(volatile long *sp)
{
    volatile long res;

    do {
        res = m_cmpswap(sp, ZEROSPININITVAL, ZEROSPINLKVAL);
    } while (res != ZEROSPININITVAL);

    return res;
}

/*
 * release spin-lock
 */
static __inline__ void
spinunlk(volatile long *sp)
{
    m_membar();
    *sp = ZEROSPININITVAL;
}

#endif /* __ZERO_SPIN_H__ */

