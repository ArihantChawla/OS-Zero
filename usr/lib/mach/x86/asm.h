#ifndef __MACH_X86_ASM_H__
#define __MACH_X86_ASM_H__

//#define frameisusr(tcb) ((tcb)->frame.cs == UTEXTSEL)

#include <stdint.h>
#include <zero/cdefs.h>
#include <mach/param.h>

#if defined(__x86_64__) || defined(__amd64__)
#include <mach/x86-64/types.h>
#else
#include <mach/ia32/types.h>
#endif

#define m_membar()   __asm__ __volatile__ ("mfence\n" : : : "memory")
/* memory read barrier */
#define m_memrdbar() __asm__ __volatile__ ("lfence\n" : : : "memory")
/* memory write barrier */
#define m_memwrbar() __asm__ __volatile__ ("sfence\n" : : : "memory")
/* delay exection */
#define m_waitspin() __asm__ __volatile__ ("pause\n"  : : : "memory")
#define m_endspin()
/* wait for an interrupt */
#define m_waitint()  __asm__ __volatile__ ("hlt\n"  : : : "memory")

/* atomic fetch and add, 16-bit version */
#define m_fetchadd16(p, val)       m_xadd16(p, val)
#define m_fetchaddu16(p, val)      m_xaddu16(p, val)
/* atomic fetch and add, 32-bit version */
#define m_fetchadd32(p, val)       m_xadd32(p, val)
#define m_fetchaddu32(p, val)      m_xaddu32(p, val)
/* atomic compare and swap byte */
#define m_cmpswapb(p, want, val)   (m_cmpxchg8(p, want, val) == want)
#define m_cmpswap32(p, want, val)  (m_cmpxchg32(p, want, val) == want)
#define m_cmpswapu32(p, want, val) (m_cmpxchgu32(p, want, val) == want)

/* atomic increment operation */
static __inline__ void
m_atominc32(volatile m_atomic32_t *p)
{
    __asm__ __volatile__ ("lock incl %0\n"
                          : "+m" (*(p))
                          :
                          : "memory");

    return;
}

/* atomic decrement operation */
static __inline__ void
m_atomdec32(volatile m_atomic32_t *p)
{
    __asm__ __volatile__ ("lock decl %0\n"
                          : "+m" (*(p))
                          :
                          : "memory");

    return;
}

/* atomic exchange operation */
static __inline__ int32_t
m_xchg32(volatile m_atomic32_t *p,
         int32_t val)
{
    int32_t res;

    __asm__ __volatile__ ("lock xchgl %0, %2\n"
                          : "+m" (*p), "=a" (res)
                          : "r" (val)
                          : "cc", "memory");

    return res;
}

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ m_atomic16_t
m_xadd16(volatile m_atomic16_t *p,
         m_atomic16_t val)
{
    __asm__ __volatile__ ("lock xaddw %1, %w0\n"
                          : "+m" (*(p)), "=a" (val)
                          :
                          : "cc", "memory");

    return val;
}

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ m_atomicu16_t
m_xaddu16(volatile m_atomicu16_t *p,
          m_atomicu16_t val)
{
    __asm__ __volatile__ ("lock xaddw %1, %w0\n"
                          : "+m" (*(p)), "=a" (val)
                          :
                          : "cc", "memory");

    return val;
}

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ int32_t
m_xadd32(volatile m_atomic32_t *p,
         int32_t val)
{
    __asm__ __volatile__ ("lock xaddl %1, %0\n"
                          : "+m" (*(p)), "=a" (val)
                          :
                          : "cc", "memory");

    return val;
}

/*
 * atomic fetch and add
 * - let *p = *p + val
 * - return original *p
 */
static __inline__ m_atomicu32_t
m_xaddu32(volatile m_atomicu32_t *p,
          m_atomicu32_t val)
{
    __asm__ __volatile__ ("lock xaddl %1, %0\n"
                          : "+m" (*(p)), "=a" (val)
                          :
                          : "cc", "memory");

    return val;
}

/*
 * atomic compare and exchange longword
 * - if *p == want, let *p = val
 * - return nonzero on success, zero on failure
 */
static __inline__ int32_t
m_cmpxchg32(volatile m_atomic32_t *p,
            int32_t want,
            int32_t val)
{
    int32_t res;

    __asm__ __volatile__("lock cmpxchgl %1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");

    return res;
}

/*
 * atomic compare and exchange unsigned longword
 * - if *p == want, let *p = val
 * - return nonzero on success, zero on failure
 */
static __inline__ m_atomicu32_t
m_cmpxchgu32(volatile m_atomicu32_t *p,
             m_atomicu32_t want,
             m_atomicu32_t val)
{
    m_atomicu32_t res;

    __asm__ __volatile__ ("lock cmpxchgl %1, %2\n"
                          : "=a" (res)
                          : "q" (val), "m" (*(p)), "0" (want)
                          : "memory");

    return res;
}

/* atomic set bit operation */
static INLINE void
m_setbit32(volatile m_atomic32_t *p, int32_t ndx)
{
    __asm__ __volatile__ ("lock btsl %1, %0\n"
                          : "=m" (*(p))
                          : "Ir" (ndx)
                          : "memory");

    return;
}

/* atomic reset/clear bit operation */
static INLINE void
m_clrbit32(volatile m_atomic32_t *p, int32_t ndx)
{
    int32_t mask = ~(INT32_C(1) << ndx);

    __asm__ __volatile__ ("lock andl %1, %0\n"
                          : "=m" (*((uint8_t *)(p) + (ndx >> 3)))
                          : "Ir" (mask));

    return;
}

/* atomic flip/toggle bit operation */
static INLINE void
m_flipbit32(volatile m_atomic32_t *p, int32_t ndx)
{
    int32_t bit = INT32_C(1) << ndx;

    __asm__ __volatile__ ("lock orl %1, %0\n"
                          : "=m" (*((uint8_t *)(p) + (ndx >> 3)))
                          : "Ir" (bit));

    return;
}

/* atomic set and test bit operation; returns the old value */
static __inline__ int32_t
m_cmpsetbit32(volatile m_atomic32_t *p, int32_t ndx)
{
    int32_t val = 0;

    if (IMMEDIATE(ndx)) {
        __asm__ __volatile__ ("lock btsl %2, %0\n"
                              "jnc 1f\n"
                              "incl %1\n"
                              "1:\n"
                              : "+m" (*(p)), "=r" (val)
                              : "i" (ndx)
                              : "cc", "memory");
    } else {
        __asm__ __volatile__ ("lock btsl %2, %0\n"
                              "jnc 1f\n"
                              "incl %1\n"
                              "1:\n"
                              : "+m" (*(p)), "=r" (val)
                              : "r" (ndx)
                              : "cc", "memory");
    }

    return val;
}

/* atomic clear bit operation */
static __inline__ int32_t
m_cmpclrbit32(volatile m_atomic32_t *p, int32_t ndx)
{
    int32_t val = 0;

    if (IMMEDIATE(ndx)) {
        __asm__ __volatile__ ("lock btrl %2, %0\n"
                              "jnc 1f\n"
                              "incl %1\n"
                              "1:\n"
                              : "+m" (*(p)), "=r" (val)
                              : "i" (ndx)
                              : "cc", "memory");
    } else {
        __asm__ __volatile__ ("lock btrl %2, %0\n"
                              "jnc 1f\n"
                              "incl %1\n"
                              "1:\n"
                              : "+m" (*(p)), "=r" (val)
                              : "r" (ndx)
                              : "cc", "memory");
    }

    return val;
}

#if defined(__GNUC__)
#define m_atomor(p, val)  __sync_or_and_fetch((p), (val))
#define m_atomand(p, val) __sync_and_and_fetch((p), (val))
#endif

/*
 * atomic compare and exchange byte
 * - if *p == want, let *p = val
 * - return original *p
 */
static __inline__ int8_t
m_cmpxchg8(volatile m_atomic64_t *p,
           int8_t want,
           int8_t val)
{
    long res;

    __asm__ __volatile__ ("lock cmpxchgb %b1, %2\n"
                          : "=a" (res)
                          : "q" (val), "m" (*(p)), "0" (want)
                          : "memory");

    return res;
}

static __inline__ int32_t
m_bsf32(m_atomicu32_t val)
{
    int32_t ret = ~0;

    __asm__ __volatile__ ("bsfl %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

static __inline__ int32_t
m_bsr32(m_atomicu32_t val)
{
    int32_t ret = ~0;

    __asm__ __volatile__ ("bsrl %1, %0\n" : "=r" (ret) : "rm" (val));

    return ret;
}

#endif /* __MACH_X86_ASM_H__ */

