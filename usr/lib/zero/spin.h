#ifndef __ZERO_SPIN_H__
#define __ZERO_SPIN_H__

#include <zero/asm.h>

typedef m_atomic_t zerospin;

#define ZEROSPININITVAL 0L
#define ZEROSPINLKVAL   1L

#define spininit(sp) (*(sp) = ZEROSPININITVAL)

/*
 * try to acquire spin-lock
 * - return non-zero on success, zero if already locked
 */
static __inline__ long
spintrylk(m_atomic_t *sp)
{
    m_atomic_t res;

    res = m_cmpswap(sp, ZEROSPININITVAL, ZEROSPINLKVAL);
    if (res == ZEROSPININITVAL) {

        return 1;
    }
    
    return 0;
}

/*
 * lock spin-lock
 */
static __inline__ void
spinlk(m_atomic_t *sp)
{
    m_atomic_t res = ZEROSPINLKVAL;

    while (res != ZEROSPININITVAL) {
        res = m_cmpswap(sp, ZEROSPININITVAL, ZEROSPINLKVAL);
    }

    return;
}

/*
 * release spin-lock
 */
static __inline__ void
spinunlk(m_atomic_t *sp)
{
    m_membar();
    *sp = ZEROSPININITVAL;

    return;
}

#endif /* __ZERO_SPIN_H__ */

