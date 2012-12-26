#ifndef __ZERO_X86_64_ASM_H__
#define __ZERO_X86_64_ASM_H__

/* API declarations */
#define m_membar()               __asm__ __volatile__ ("" : : : "memory")
#define m_waitint()              __asm__ __volatile__ ("pause")
#define m_cmpswap(p, want, val)  m_cmpxchgq(p, want, val)
#define m_cmpswapb(p, want, val) m_cmpxchgb(p, want, val)
#define m_fetadd(p, val)         xaddq(p, val)
#define m_getretadr(r)                                                  \
    __asm__ __volatile__ ("movl 8(%%rbp), %0" : "=r" (r))

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ long
m_xaddq(long *p,
        long val)
{
    __asm__ __volatile__ ("lock xaddq %%rax, %b2\n"
                          : "=a" (val)
                          : "a" (val), "m" (*(p))
                          : "memory");

    return val;
}

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

/*
 * atomic compare and exchange longword
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ uint8_t
m_cmpxchgq(long *p,
           long want,
           long val)
{
    long res;
    
    __asm__ __volatile__("lock cmpxchgq %1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");
    
    return res;
}

#endif /* __ZERO_X86_64_ASM_H__ */

