#ifndef __ZERO_IA32_ASM_H__
#define __ZERO_IA32_ASM_H__

#include <zero/x86/asm.h>

/* API declarations */
#define m_xchg(p, val)          m_xchgl(p, val)
#define m_fetadd(p, val)        m_xaddl(p, val)
#define m_cmpswap(p, want, val) m_cmpxchgl(p, want, val)
#define m_scanlo1bit(l)         m_bsfl(l)
#define m_scanhi1bit(l)         m_bsrl(l)
#if !defined(__GNUC__)
static __inline__ void
m_getretadr(void **pp) {
    void *_ptr;
    
    __asm__ __volatile__ ("movl 4(%%ebp), %0\n" : "=rm" (_ptr));
    *pp = _ptr;

    return;
}

static __inline__ void
m_getfrmadr(void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movl %%ebp, %0\n" : "=rm" (_ptr));
    *pp = _ptr;

    return;
}
#endif

static __inline__ void
m_loadretadr(void *frm, void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movl 4(%1), %0\n" : "=rm" (_ptr) : "r" (frm));
    *pp = _ptr;

    return;
}

static __inline__ void
m_getclrfrmadr(void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movl *%%ebp, %0\n" : "=rm" (_ptr));
    *pp = _ptr;

    return;
}

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ long
m_xaddl(volatile long *p,
        long val)
{
    __asm__ __volatile__ ("lock xaddl %%eax, %2\n"
                          : "=a" (val)
                          : "a" (val), "m" (*(p))
                          : "memory");

    return val;
}

static __inline__ long
m_xchgl(volatile long *p, long val)
{
    volatile long res;

    __asm__ __volatile__ ("lock xchgl %0, %2\n"
                          : "+m" (*p), "=a" (res)
                          : "r" (val)
                          : "cc");

    return res;
}

/*
 * atomic compare and exchange longword
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ long
m_cmpxchgl(volatile long *p,
           long want,
           long val)
{
    volatile long res;
    
    __asm__ __volatile__("lock cmpxchgl %1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");
    
    return res;
}

static __inline__ unsigned long
m_bsfl(unsigned long val)
{
    unsigned long ret;

    __asm__ __volatile__ ("bsfl %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

static __inline__ unsigned long
m_bsrl(unsigned long val)
{
    unsigned long ret;

    __asm__ __volatile__ ("bsrl %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

#endif /* __ZERO_IA32_ASM_H__ */

