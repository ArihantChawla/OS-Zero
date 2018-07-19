/*
 * reference counting
 */

#include <zero/asm.h>

/* increment reference count by one; return new count */
static __inline__ long
refinc(volatile long *lp)
{
    long res = m_fetchadd(lp, 1);

    res++;

    return res;
}

/* decrement reference count by one; return new count */
static __inline__ long
refdec(volatile long *lp)
{
    long res = m_fetchadd(lp, -1);

    res--;

    return res;
}

/* TODO: I wonder if refadd() and refsub() will be of any use... =) */

static __inline__ long
refadd(volatile long *lp,
       long n)
{
    long res = m_fetchadd(lp, n);

    res += n;

    return res;
}

static __inline__ long
refsub(volatile long *lp,
       long n)
{
    long res = m_fetchadd(lp, -n);

    res -= n;

    return res;
}

