#ifndef __MACH_IA32_ASM_H__
#define __MACH_IA32_ASM_H__

/* API declarations */
#define m_membar()               __asm__ __volatile__ ("" : : : "memory")
#define m_waitint()              __asm__ __volatile__ ("pause")
#define m_cmpswap(p, want, val)  m_cmpxchgl(p, want, val)
#define m_cmpswapb(p, want, val) m_cmpxchgb(p, want, val)
#define m_fetadd(p, val)         xaddl(p, val)

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ long
m_xaddl(long *p,
        long val)
{
    __asm__ __volatile__ ("lock xaddl %%eax, %b2\n"
                          : "=a" (val)
                          : "a" (val), "m" (*(p))
                          : "memory");

    return val;
}

static __inline__ long
m_xchgl(volatile long *adr, long val)
{
    long res;

    __asm__ __volatile__ ("lock; xchgl %0, %2\n"
                          : "+m" (*adr), "=a" (res)
                          : "r" (val)
                          : "cc");

    return res;
}

#if 0
/*
 * atomic compare and exchange byte
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ uint8_t
m_cmpxchgb(long *p,
           long want,
           long val)
{
    long res;

    __asm__ __volatile__("lock cmpxchgb %b1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");

    return res;
}
#endif

/*
 * atomic compare and exchange longword
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ long
m_cmpxchgl(long *p,
           long want,
           long val)
{
    long res;
    
    __asm__ __volatile__("lock cmpxchgl %1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");
    
    return res;
}

#endif /* __MACH_IA32_ASM_H__ */

