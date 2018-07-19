#ifndef __MACH_IA32_ASM_H__
#define __MACH_IA32_ASM_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>

extern uint32_t asmgetpc(void);

#define m_atominc(p)                 m_atominc32(p)
#define m_atomdec(p)                 m_atomdec32(p)
#define m_atomswap(p, val)           m_xchg32(p, val)
#define m_fetchadd(p, val)           m_xadd32(p, val)
#define m_fetchswap(p, want, val)    m_cmpxchg32(p, want, val)
#define m_fetchswapu(p, want, val)   m_cmpxchgu32(p, want, val)
#define m_fetchswapptr(p, want, val) m_cmpxchg32ptr(p, want, val)
#define m_cmpswap(p, want, val)      (m_cmpxchg32(p, want, val) == want)
#define m_cmpswapu(p, want, val)     (m_cmpxchgu32(p, want, val) == want)
#define m_cmpswapptr(p, want, val)   (m_cmpxchg32ptr(p, want, val) == want)
#define m_cmpswapdbl(p, want, val)   (m_cmpxchg64(p, val))
#define m_setbit(p, ndx)             m_setbit32(p, ndx)
#define m_clrbit(p, ndx)             m_clrbit32(p, ndx)
#define m_flipbit(p, ndx)            m_flipbit32(p, ndx)
#define m_cmpsetbit(p, ndx)          m_cmpsetbit32(p, ndx)
#define m_cmpclrbit(p, ndx)          m_cmpclrbit32(p, ndx)
#define m_scanlo1bit(l)              m_bsf32(l)
#define m_scanhi1bit(l)              m_bsr32(l)

#define __EIPFRAMEOFS                offsetof(struct m_stkframe, pc)

static INLINE void *
m_getretadr(void **pp)
{
    void *ptr;

    __asm__ __volatile__ ("movl %c1(%%ebp), %0\n"
                          : "=r" (ptr)
                          : "i" (__EIPFRAMEOFS));
    return ptr;
}

static INLINE void *
m_getfrmadr(void)
{
    void *ptr;

    __asm__ __volatile__ ("movl %%ebp, %0\n" : "=r" (ptr));

    return ptr;
}

static INLINE void
m_getfrmadr2(void *fp, void **pp)
{
    void *ptr;

    __asm__ __volatile__ ("movl %1, %%eax\n"
                          "movl (%%eax), %0\n"
                          : "=r" (ptr)
                          : "rm" (fp)
                          : "eax");
    *pp = ptr;

    return;
}

static INLINE void
m_loadretadr(void *frm,
             void **pp)
{
    void *ptr;

    __asm__ __volatile__ ("movl %c1(%2), %0\n"
                          : "=r" (ptr)
                          : "i" (__EIPFRAMEOFS), "r" (frm));
    *pp = ptr;

    return;
}

/*
 * atomic compare and exchange pointer
 * - if *p == want, let *p = val
 * return nonzero on success, zero on failure
 */
static __inline__ void *
m_cmpxchg32ptr(m_atomic32_t **p,
               m_atomic32_t *want,
               m_atomicptr_t *val)
{
    void *res;

    __asm__ __volatile__("lock cmpxchgl %1, %2\n"
                         : "=a" (res)
                         : "q" (val), "m" (*(p)), "0" (want)
                         : "memory");

    return res;
}

#if defined(__GNUC__) && 0

/*
 * atomic 64-bit compare and swap
 * - if *p == want, let *p = val
 * - return original nonzero on success, zero on failure
 */
static __inline__ long
m_cmpxchg64(int64_t *p64,
            int64_t *want,
            int64_t *val)
{
    return __sync_bool_compare_and_swap(p64, want, val);
}

#elif defined(_MSC_VER)

static __inline__ long
m_cmpxchg64(int64_t *p64,
            int64_t *want,
            int64_t *val)
{
    long long ask = *want;
    long long say = *val;
    long long res;

    res = InterlockedCompareExchange64(p64, say, ask);

    return res;
}

#else

/*
 * atomic 64-bit compare and swap
 * - if *p == want, let *p = val + set ZF
 *   - else let EDX:EAX = *p + clear ZF
 * - return 0 on failur, 1 on success

 */
static __inline__ long
m_cmpxchg64(int64_t *ptr,
            int64_t want,
            int64_t val)
{
    long     res = 0;
    register int32_t ebx __asm__ ("ebx") = want & 0xffffffff;

    __asm__ __volatile (
        "movl %%edi, %%ebx\n" // load EBX
        "lock cmpxchg8b (%%esi)\n"
        "setz %%al\n" // EAX may change
        "movzx %%al, %1\n" // res = ZF
        : "+S" (ptr), "=a" (res)
        : "0" (ptr),
          "d" ((uint32_t)(want >> 32)),
          "a" ((uint32_t)(want & 0xffffffff)),
          "c" ((uint32_t)(val >> 32)),
          "D" ((uint32_t)(val & 0xffffffff))
        : "flags", "memory", "eax", "edx");

    return res;
}

#endif

#endif /* __MACH_IA32_ASM_H__ */

