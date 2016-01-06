#ifndef __ZERO_TKTLK_H__
#define __ZERO_TKTLK_H__

/* REFERENCE: http://locklessinc.com/articles/locks/ */

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/asm.h>
#include <zero/thr.h>

#if (LONGSIZE == 4)

typedef union zerotktlk {
    volatile unsigned int uval;
    union {
        unsigned short    val;
        unsigned short    nref;
    }
} zerotktlk;

#elif (LONGSIZE == 8)

typedef union zerotktlk {
    volatile unsigned long uval;
    union {
        unsigned int       val;
        unsigned int       nref;
    }
} zerotktlk;

#endif /* LONGSIZE */

#if (LONGSIZE == 4)

static INLINE void
tktlk(zerotktlk *tp)
{
    volatile unsigned short val = m_fetchadd16(&tp->nref, 1);

    while (tp->val != val) {
        thryield();
    }

    return;
}

static INLINE void
tktunlk(zerotktlk *tp)
{
    m_membar();
    tp->val++;

    return;
}

/* return 1 if lock succeeds, 0 otherwise */
static INLINE long
tkttrylk(zerotktlk *tp)
{
    volatile unsigned short val = tp->nref;
    unsigned short          next = val + 1;
    unsigned int            cmp = (val << 16) | val;
    unsigned int            cmpnew = (next << 16) | val;
    long                    res = 0;

    if (m_cmpswap(&tp->uval, cmp, cmpnew)) {
        res++;
    }

    return res;
}

#elif (LONGSIZE == 8)

static INLINE void
tktlk(zerotktlk *tp)
{
    volatile unsigned long val = m_fetchadd32(&tp->nref, 1);

    while (tp->val != val) {
        thryield();
    }

    return;
}

static INLINE void
tktunlk(zerotktlk *tp)
{
    m_membar();
    tp->val++;

    return;
}

/* return 1 if lock succeeds, 0 otherwise */
static INLINE long
tkttrylk(zerotktlk *tp)
{
    volatile unsigned long val = tp->nref;
    unsigned long          next = val + 1;
    unsigned long          cmp = (val << 32) | val;
    unsigned long          cmpnew = (next << 32) | val;
    long                   res = 0;

    if (m_cmpswap(&tp->uval, cmp, cmpnew)) {
        res++;
    }

    return res;
}

#endif /* LONGSIZE */

#endif /* __ZERO_TKTLK_H__ */

