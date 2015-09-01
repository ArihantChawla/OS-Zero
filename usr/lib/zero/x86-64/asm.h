#ifndef __ZERO_X86_64_ASM_H__
#define __ZERO_X86_64_ASM_H__

#include <zero/x86/asm.h>

/* API declarations */
#define m_xchg(p, val)           m_xchgq(p, val)
#define m_fetadd(p, val)         m_xaddq(p, val)
#define m_cmpswap(p, want, val)  m_cmpxchgq(p, want, val)
#define m_scanlo1bit(l)          m_bsfq(l)
#define m_scanhi1bit(l)          m_bsrq(l)
#if defined(__GNUC__) && 0
#define m_setretadr(pp)                                                 \
    (*(pp) = (void *)__builtin_frob_return_address(__builtin_return_address(0)))
#define m_getretadr(pp)                                                 \
    (*(pp) = (void *)__builtin_extract_return_address(__builtin_return_address(0)))
#define m_getfrmadr(pp)                                                 \
    (*(pp) = (void *)__builtin_frame_address(0))
#else
static __inline__ void
m_getretadr(void **pp) {
    void *_ptr;
    
    __asm__ __volatile__ ("movq 8(%%rbp), %0\n" : "=rm" (_ptr));
    *pp = _ptr;

    return;
}
#endif


static __inline__ long
m_xchgq(volatile long *p,
        long val)
{
    __asm__ __volatile__ ("lock xchgq %%rax, %q1\n"
                          : "=a" (val)
                          : "m" (*(p))
                          : "memory");
    
    return val;
}

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ long
m_xaddq(volatile long *p,
        long val)
{
    __asm__ __volatile__ ("lock xaddq %%rax, %q2\n"
                          : "=a" (val)
                          : "a" (val), "m" (*(p))
                          : "memory");

    return val;
}

/*
 * atomic compare and exchange longword
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ char
m_cmpxchgq(volatile long *p,
           long want,
           long val)
{
    volatile long res;
    
    __asm__ __volatile__ ("lock cmpxchgq %1, %2\n"
                          : "=a" (res)
                          : "q" (val), "m" (*(p)), "0" (want)
                          : "memory");
    
    return res;
}

static __inline__ unsigned long
m_bsfq(unsigned long val)
{
    unsigned long ret;

    __asm__ __volatile__ ("bsfq %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

static __inline__ unsigned long
m_bsrq(unsigned long val)
{
    unsigned long ret;

    __asm__ __volatile__ ("bsrq %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

#endif /* __ZERO_X86_64_ASM_H__ */

