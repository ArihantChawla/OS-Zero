#ifndef __ZERO_TKTLK_H__
#define __ZERO_TKTLK_H__

/* REFERENCE: http://locklessinc.com/articles/locks/ */

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/asm.h>
#define PTHREAD 1
#include <zero/thr.h>

#if (LONGSIZE == 4)

union zerotktlk {
    volatile unsigned int uval;
    struct {
        unsigned short    val;
        unsigned short    nref;
    } s;
};

#elif (LONGSIZE == 8)

union zerotktlk {
    volatile unsigned long uval;
    struct {
        unsigned int       val;
        unsigned int       nref;
    } s;
};

#endif /* LONGSIZE */

typedef union zerotktlk zerotktlk;

#if (LONGSIZE == 4)

static INLINE void
tktlk(union zerotktlk *tp)
{
    volatile unsigned short val = m_fetchaddu16(&tp->s.nref, 1);
    
    while (tp->s.val != val) {
        thryield();
    }
    
    return;
}

static INLINE void
tktunlk(union zerotktlk *tp)
{
    m_membar();
    tp->s.val++;
    
    return;
}

/* return 1 if lock succeeds, 0 otherwise */
static INLINE long
tkttrylk(union zerotktlk *tp)
{
    volatile unsigned short val = tp->s.nref;
    unsigned short          next = val + 1;
    unsigned int            cmp = (val << 16) | val;
    unsigned int            cmpnew = (next << 16) | val;
    long                    res = 0;
    
    if (m_cmpswapu(&tp->uval, cmp, cmpnew) == cmp) {
        res++;
    }
    
    return res;
}

#elif (LONGSIZE == 8)

static INLINE void
tktlk(union zerotktlk *tp)
{
    volatile unsigned long val;

    val = m_fetchaddu32(&tp->s.nref, 1);
    while (tp->s.val != val) {
        thryield();
    }

    return;
}

static INLINE void
tktunlk(union zerotktlk *tp)
{
    m_membar();
    tp->s.val++;

    return;
}

/* return 1 if lock succeeds, 0 otherwise */
static INLINE long
tkttrylk(union zerotktlk *tp)
{
    volatile unsigned long val = tp->s.nref;
    unsigned long          next = val + 1;
    unsigned long          cmp = (val << 32) | val;
    unsigned long          cmpnew = (next << 32) | val;
    long                   res = 0;

    if (m_cmpswapu(&tp->uval, cmp, cmpnew) == cmp) {
        res++;
    }

    return res;
}

#endif /* LONGSIZE */

#endif /* __ZERO_TKTLK_H__ */

