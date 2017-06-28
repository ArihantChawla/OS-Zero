#ifndef __ZERO_TKTLK_H__
#define __ZERO_TKTLK_H__

/* REFERENCE: http://locklessinc.com/articles/locks/ */

#include <stdint.h>
#include <endian.h>
#include <zero/param.h>
#include <zero/cdefs.h>
#include <zero/asm.h>
#define PTHREAD 1
#include <zero/thr.h>

#if !defined(TKTLKSIZE)
#define TKTLKSIZE  4
#endif
#define TKTLKNSPIN 16384

#if (TKTLKSIZE == 4)

union zerotktlk {
    volatile unsigned int uval;
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
    struct {
        unsigned short    val;
        unsigned short    nref;
    } s;
#else
    struct {
        unsigned short    nref;
        unsigned short    val;
    } s;
#endif
};

#elif (TKTLKSIZE == 8)

union zerotktlk {
    volatile unsigned long uval;
#if (__BYTE_ORDER == __LITTLE_ENDIAN)
    struct {
        unsigned int       val;
        unsigned int       nref;
    } s;
#else
    struct {
        unsigned int       nref;
        unsigned int       val;
    } s;
#endif
};

#endif /* TKTLKSIZE */

typedef union zerotktlk zerotktlk;

#if (TKTLKSIZE == 4)

static INLINE void
tktlkspin(union zerotktlk *tp)
{
    volatile unsigned short val = m_fetchaddu16(&tp->s.nref, 1);
    long                    nspin = TKTLKNSPIN;
    
    do {
        nspin--;
    } while ((nspin) && tp->s.val != val);
    while (tp->s.val != val) {
        m_waitspin();
    }
        
    return;
}
    
static INLINE void
tktlk(union zerotktlk *tp)
{
    volatile unsigned short val = m_fetchaddu16(&tp->s.nref, 1);
    
    while (tp->s.val != val) {
        m_waitspin();
    }
    
    return;
}

static INLINE void
tktunlk(union zerotktlk *tp)
{
    m_membar();
    tp->s.val++;
    m_endspin();
    
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
    
    if (m_cmpswapu32(&tp->uval, cmp, cmpnew)) {
        res++;
    }
    
    return res;
}

#elif (TKTLKSIZE == 8)

static INLINE void
tktlkspin(union zerotktlk *tp)
{
    volatile unsigned long val = m_fetchaddu32(&tp->s.nref, 1);
    long                   nspin = TKTLKNSPIN;
    
    do {
        nspin--;
    } while ((nspin) && tp->s.val != val);
    while (tp->s.val != val) {
        m_waitspin();
    }

    return;
}

static INLINE void
tktlk(union zerotktlk *tp)
{
    volatile unsigned long val = m_fetchaddu32(&tp->s.nref, 1);

    while (tp->s.val != val) {
        m_waitspin();
    }

    return;
}

static INLINE void
tktunlk(union zerotktlk *tp)
{
    m_membar();
    tp->s.val++;
    m_endspin();

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

    if (m_cmpswapu64(&tp->uval, cmp, cmpnew)) {
        res++;
    }

    return res;
}

#endif /* TKTLKSIZE */

static INLINE long
tktmaylk(union zerotktlk *tp)
{
    union zerotktlk tu = *tp;

    m_membar();

    return (tu.s.val == tu.s.nref);
}

#endif /* __ZERO_TKTLK_H__ */

