#ifndef __ZERO_X86_ASM_H__
#define __ZERO_X86_ASM_H__

/* memory barrier */
#define m_membar()   __asm__ __volatile__ ("mfence\n" : : : "memory")
/* memory read barrier */
#define m_memrdbar() __asm__ __volatile__ ("lfence\n" : : : "memory")
/* memory write barrier */
#define m_memwrbar() __asm__ __volatile__ ("sfence\n" : : : "memory")
/* wait for an interrupt */
#define m_waitint()  __asm__ __volatile__ ("pause\n"  : : : "memory")
/* atomic fetch and add, 32-bit version */
#define m_fetchadd32(p, val)     m_xadd32((volatile long *)(p), val)
/* atomic compare and swap byte */
#define m_cmpswapb(p, want, val) m_cmpxchg8(p, want, val)

/* atomic increment operation */
static __inline__ void
m_atominc32(volatile long *p)
{
    __asm__ __volatile__ ("lock incl %0\n"
                          : "+m" (*(p)));
}

/* atomic decrement operation */
static __inline__ void
m_atomdec32(volatile long *p)
{
    __asm__ __volatile__ ("lock decl %0\n"
                          : "+m" (*(p)));
}

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ long
m_xadd32(volatile long *p,
         long val)
{
    volatile int *_ptr = (volatile int *)p;
    
    __asm__ __volatile__ ("lock xaddq %%rax, %2\n"
                          : "=a" (val)
                          : "a" (val), "m" (*(_ptr))
                          : "memory");

    return val;
}

/*
 * atomic compare and exchange byte
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ char
m_cmpxchg8(volatile long *p,
           long want,
           long val)
{
    volatile long res;

    __asm__ __volatile__ ("lock cmpxchgb %b1, %2\n"
                          : "=a" (res)
                          : "q" (val), "m" (*(p)), "0" (want)
                          : "memory");

    return (char)res;
}

#endif /* __ZERO_X86_ASM_H__ */


