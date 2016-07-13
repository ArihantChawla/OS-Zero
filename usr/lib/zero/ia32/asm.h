#ifndef __ZERO_IA32_ASM_H__
#define __ZERO_IA32_ASM_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/types.h>
#include <zero/x86/asm.h>

extern uint32_t asmgetpc(void);

#define m_atominc(p)               m_atominc32(p)
#define m_atomdec(p)               m_atomdec32(p)
#define m_swap(p, val)             m_xchg32(p, val)
#define m_fetchadd(p, val)         m_xadd32(p, val)
#define m_cmpswap(p, want, val)    m_cmpxchg32(p, want, val)
#define m_cmpswapptr(p, want, val) m_cmpxchg32ptr(p, want, val)
#define m_cmpsetbit(p, ndx)        m_cmpsetbit32(p, ndx)
#define m_cmpclrbit(p, ndx)        m_cmpclrbit32(p, ndx)
#define m_scanlo1bit(l)            m_bsf32(l)
#define m_scanhi1bit(l)            m_bsr32(l)

#define __EIPFRAMEOFS              offsetof(struct m_stkframe, pc)

static INLINE void
m_getretadr(void **pp)
{
    void *_ptr;
    
    __asm__ __volatile__ ("movl %c1(%%ebp), %0\n"
                          : "=r" (_ptr)
                          : "i" (__EIPFRAMEOFS));
    *pp = _ptr;

    return;
}

static INLINE void
m_getfrmadr(void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movl %%ebp, %0\n" : "=r" (_ptr));
    *pp = _ptr;

    return;
}

static INLINE void
m_getfrmadr2(void *fp, void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movl %1, %%eax\n"
                          "movl (%%eax), %0\n"
                          : "=r" (_ptr)
                          : "rm" (fp)
                          : "eax");
    *pp = _ptr;

    return;
}

static INLINE void
m_loadretadr(void *frm,
             void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movl %c1(%2), %0\n"
                          : "=r" (_ptr)
                          : "i" (__EIPFRAMEOFS), "r" (frm));
    *pp = _ptr;

    return;
}

static INLINE void
m_getretfrmadr(void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movl (%%ebp), %0\n" : "=r" (_ptr));
    *pp = _ptr;

    return;
}

/*
 * atomic compare and exchange pointer
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ volatile void *
m_cmpxchg32ptr(volatile long *p,
               volatile long *want,
               volatile void *val)
{
    volatile void *res;
    
    __asm__ __volatile__("lock cmpxchgl %1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");
    
    return res;
}

#endif /* __ZERO_IA32_ASM_H__ */

