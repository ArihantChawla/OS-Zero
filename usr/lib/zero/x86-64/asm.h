#ifndef __ZERO_X86_64_ASM_H__
#define __ZERO_X86_64_ASM_H__

#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/x86/asm.h>

extern uint64_t asmgetpc(void);

#define m_atominc(p)            m_atominc64(p)
#define m_atomdec(p)            m_atomdec64(p)
#define m_xchg(p, val)          m_xchg64(p, val)
#define m_fetchadd(p, val)      m_xadd64(p, val)
#define m_cmpswap(p, want, val) m_cmpxchg64(p, want, val)
#define m_cmpsetbit(p, ndx)     m_cmpsetbit64(p, ndx)
#define m_cmpclrbit(p, ndx)     m_cmpclrbit64(p, ndx)
#define m_scanlo1bit(l)         m_bsf64(l)
#define m_scanhi1bit(l)         m_bsr64(l)

static INLINE void
m_getretadr(void **pp)
{
    void *_ptr;
    
    __asm__ __volatile__ ("movq 8(%%rbp), %0\n" : "=r" (_ptr));
    *pp = _ptr;

    return;
}

static INLINE void
m_getfrmadr(void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movq %%rbp, %0\n" : "=r" (_ptr));
    *pp = _ptr;

    return;
}

static INLINE void
m_getfrmadr2(void *fp, void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movq %1, %%rax\n"
                          "movq (%%rax), %0\n"
                          : "=r" (_ptr)
                          : "rm" (fp)
                          : "rax");
    *pp = _ptr;

    return;
}

static INLINE void
m_loadretadr(void *frm, void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movq 8(%1), %0\n" : "=r" (_ptr) : "r" (frm));
    *pp = _ptr;

    return;
}

static INLINE void
m_getretfrmadr(void **pp)
{
    void *_ptr;

    __asm__ __volatile__ ("movq (%%rbp), %0\n" : "=r" (_ptr));
    *pp = _ptr;

    return;
}

/* atomic increment operation */
static __inline__ void
m_atominc64(volatile long *p)
{
    __asm__ __volatile__ ("lock incq %0\n"
                          : "+m" (*(p)));
}

/* atomic decrement operation */
static __inline__ void
m_atomdec64(volatile long *p)
{
    __asm__ __volatile__ ("lock decq %0\n"
                          : "+m" (*(p)));
}

static __inline__ long
m_xchg64(volatile long *p,
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
m_xadd64(volatile long *p,
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
static __inline__ long
m_cmpxchg64(volatile long *p,
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

/* atomic set bit operation */
static __inline__ long
m_cmpsetbit64(volatile long *p, long ndx)
{
    volatile long val = 0;

    __asm__ __volatile__ ("lock btsq %c2, %0\n"
                          "jnc 1f\n"
                          "movq $0x01, %1\n"
                          "1:\n"
                          : "=m" (*(p)), "=r" (val)
                          : "r" (ndx)
                          : "memory");

    return val;
}

/* atomic clear bit operation */
static __inline__ long
m_cmpclrbit64(volatile long *p, long ndx)
{
    volatile long val = 0;

    __asm__ __volatile__ ("lock btrq %c2, %0\n"
                          "jnc 1f\n"
                          "movq $0x01, %1\n"
                          "1:\n"
                          : "=m" (*(p)), "=r" (val)
                          : "r" (ndx)
                          : "memory");

    return val;
}

static __inline__ unsigned long
m_bsf64(unsigned long val)
{
    unsigned long ret;

    __asm__ __volatile__ ("bsfq %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

static __inline__ unsigned long
m_bsr64(unsigned long val)
{
    unsigned long ret;

    __asm__ __volatile__ ("bsrq %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

#endif /* __ZERO_X86_64_ASM_H__ */

