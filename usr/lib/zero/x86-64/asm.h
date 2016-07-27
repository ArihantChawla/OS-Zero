#ifndef __ZERO_X86_64_ASM_H__
#define __ZERO_X86_64_ASM_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/types.h>
#include <zero/x86/asm.h>

extern uint64_t asmgetpc(void);

#define m_atominc(p)               m_atominc64(p)
#define m_atomdec(p)               m_atomdec64(p)
#define m_swap(p, val)             m_xchg64(p, val)
#define m_fetchadd(p, val)         m_xadd64(p, val)
#define m_fetchaddu(p, val)        m_xaddu64(p, val)
#define m_cmpswap(p, want, val)    m_cmpxchg64(p, want, val)
#define m_cmpswapu(p, want, val)   m_cmpxchgu64(p, want, val)
#define m_cmpswapptr(p, want, val) m_cmpxchg64ptr(p, want, val)
#define m_cmpswapdbl(p, want, val) m_cmpxchg128(p, want, val)
#define m_cmpsetbit(p, ndx)        m_cmpsetbit64(p, ndx)
#define m_cmpclrbit(p, ndx)        m_cmpclrbit64(p, ndx)
#define m_scanlo1bit(l)            m_bsf64(l)
#define m_scanhi1bit(l)            m_bsr64(l)

#define __RIPFRAMEOFS              offsetof(struct m_stkframe, pc)

static INLINE void
m_getretadr(void **pp)
{
    void *_ptr;
    
    __asm__ __volatile__ ("movq %c1(%%rbp), %0\n"
                          : "=r" (_ptr)
                          : "i" (__RIPFRAMEOFS));
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

    __asm__ __volatile__ ("movq %c1(%2), %0\n"
                          : "=r" (_ptr)
                          : "i" (__RIPFRAMEOFS), "r" (frm));
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
                          : "+m" (*(p))
                          :
                          : "memory");
}

/* atomic decrement operation */
static __inline__ void
m_atomdec64(volatile long *p)
{
    __asm__ __volatile__ ("lock decq %0\n"
                          : "+m" (*(p))
                          :
                          : "memory");
}

static __inline__ long
m_xchg64(volatile long *p,
         long val)
{
    __asm__ __volatile__ ("lock xchgq %%rax, %q1\n"
                          : "=a" (val)
                          : "m" (*(p))
                          : "cc", "memory");
    
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
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ unsigned long
m_xaddu64(volatile unsigned long *p,
          unsigned long val)
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

static __inline__ unsigned long
m_cmpxchgu64(volatile unsigned long *p,
             unsigned long want,
             unsigned long val)
{
    volatile unsigned long res;
    
    __asm__ __volatile__ ("lock cmpxchgq %1, %2\n"
                          : "=a" (res)
                          : "q" (val), "m" (*(p)), "0" (want)
                          : "memory");
    
    return res;
}

/*
 * atomic compare and exchange pointer
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ volatile void *
m_cmpxchg64ptr(volatile long *p,
               volatile long *want,
               volatile void *val)
{
    volatile void *res;
    
    __asm__ __volatile__("lock cmpxchgq %1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");
    
    return res;
}

#if defined(__GNUC__) && 0

/*
 * atomic 128-bit compare and swap
 * - if *p == want, let *p = val
 * - return original nonzero on success, zero on failure
 */
static __inline__ long
m_cmpxchg128(volatile long *p64,
             volatile long *want,
             volatile long *val)
{
    return __sync_bool_compare_and_swap(p64, want, val);
}

#elif defined(_MSC_VER)

static __inline__ long
m_cmpxchg128(volatile int64_t *p64,
             volatile int64_t *want,
             volatile int64_t *val)
{
    __i64 lo = val[0];
    __i64 hi = val[1];

    return InterlockedCompareExchange128(p64, hi, lo, want);
}

#else

/*
 * atomic 128-bit compare and swap
 * - if *p == want, let *p = val
 * - return original nonzero on success, zero on failure
 */
static __inline__ long
m_cmpxchg128(volatile long *p64,
             volatile long *want,
             volatile long *val)
{
    uint64_t rax = p64[0];
    uint64_t rdx = p64[1];
    uint64_t val0 = val[0];
    uint64_t val1 = val[1];
    long     res;
    
    __asm__ __volatile__ ("lock cmpxchg16b %1\n"
                          "setz %0\n"
                          : "=q" (res), "+m" (*val), "+a" (rax), "+d" (rdx)
                          : "b" (val0), "c" (val1)
                          : "cc", "memory");

    return res;
}

#endif

/* atomic set and test bit operation; returns the old value */
static __inline__ long
m_cmpsetbit64(volatile long *p, long ndx)
{
    volatile long val;

    __asm__ __volatile__ ("lock btsq %2, %0\n"
                          "jnc 1f\n"
                          "movq $0x01, %1\n"
                          "1:\n"
                          "xorq %1, %1"
                          : "=m" (*(p)), "=r" (val)
                          : "r" (ndx)
                          : "memory");

    return val;
}

/* atomic clear bit operation */
static __inline__ long
m_cmpclrbit64(volatile long *p, long ndx)
{
    volatile long val;

    __asm__ __volatile__ ("lock btrq %2, %0\n"
                          "jnc 1f\n"
                          "movq $0x01, %1\n"
                          "1:\n"
                          "xorq %1, %1\n"
                          : "=m" (*(p)), "=r" (val)
                          : "r" (ndx)
                          : "memory");

    return val;
}

static __inline__ unsigned long
m_bsf64(unsigned long val)
{
    unsigned long ret = ~0UL;

    __asm__ __volatile__ ("bsfq %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

static __inline__ unsigned long
m_bsr64(unsigned long val)
{
    unsigned long ret = ~0UL;

    __asm__ __volatile__ ("bsrq %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

#endif /* __ZERO_X86_64_ASM_H__ */

