#ifndef __ZERO_TRIX_H__
#define __ZERO_TRIX_H__

/*
 * this file contains tricks I've gathered together from sources such as MIT
 * HAKMEM and the book Hacker's Delight
 */

#define ZEROABS 1

#include <stdint.h>
#include <limits.h>
#include <zero/param.h>
#include <zero/asm.h>

/* get the lowest 1-bit in a */
#define lo1bit(a)           ((a) & -(a))
/* get n lowest and highest bits of i */
#define lobits(i, n)        ((i) & ((1U << (n)) - 0x01))
#define hibits(i, n)        ((i) & ~((1U << (sizeof(i) * CHAR_BIT - (n))) - 0x01))
/* get n bits starting from index j */
#define getbits(i, j, n)    (lobits((i) >> (j), (n)))
/* set n bits starting from index j to value b */
#define setbits(i, j, n, b) ((i) |= (((b) << (j)) & ~(((1U << (n)) << (j)) - 0x01)))
#define bitset(p, b)        (((uint8_t *)(p))[(b) >> 3] & (1U << ((b) & 0x07)))
/* set bit # b in *p */
#define setbit(p, b)        (((uint8_t *)(p))[(b) >> 3] |= (1U << ((b) & 0x07)))
/* clear bit # b in *p */
#define clrbit(p, b)        (((uint8_t *)(p))[(b) >> 3] &= ~(1U << ((b) & 0x07)))
/* m - mask of bits to be taken from b. */
#define mergebits(a, b, m)  ((a) ^ (((a) ^ (b)) & (m)))
/* m - mask of bits to be copied from a. 1 -> copy, 0 -> leave alone. */
#define copybits(a, b, m) (((a) | (m)) | ((b) & ~(m)))

/* compute minimum and maximum of a and b without branching */
#define min(a, b)                                                       \
    ((b) + (((a) - (b)) & -((a) < (b))))
#define max(a, b)                                                       \
    ((a) - (((a) - (b)) & -((a) < (b))))
/* compare with power-of-two p2 */
#define gtpow2(u, p2)  /* true if u > p2 */                             \
    ((u) & ~(p2))
#define gtepow2(u, p2) /* true if u >= p2 */                            \
    ((u) & -(p2))
/* swap a and b without a temporary variable */
#define swap(a, b)     ((a) ^= (b), (b) ^= (a), (a) ^= (b))
/* compute absolute value of integer without branching; PATENTED in USA :( */
#if (ZEROABS)
#define zeroabs(a)                                                      \
    (((a) ^ (((a) >> (CHAR_BIT * sizeof(a) - 1))))                      \
     - ((a) >> (CHAR_BIT * sizeof(a) - 1)))
#define abs(a)         zeroabs(a)
#define labs(a)        zeroabs(a)
#define llabs(a)       zeroabs(a)
#endif

/* true if x is a power of two */
#define powerof2(x)     (!((x) & ((x) - 1)))
/* align a to boundary of (the power of two) b2. */
//#define align(a, b2)   ((a) & ~((b2) - 1))
//#define align(a, b2)    ((a) & -(b2))
#define modpow2(a, b2)     ((a) & ((b2) - 1))

/* round a up to the next multiple of (the power of two) b2. */
//#define roundup2a(a, b2) (((a) + ((b2) - 0x01)) & ~((b2) + 0x01))
#define rounduppow2(a, b2) (((a) + ((b2) - 0x01)) & -(b2))

/* round down to the previous multiple of (the power of two) b2 */
#define rounddownpow2(a, b2) ((a) & ~((b2) - 0x01))

/* compute the average of a and b without division */
#define uavg(a, b)      (((a) & (b)) + (((a) ^ (b)) >> 1))

#define divceil(a, b)   (((a) + (b) - 1) / (b))
#define divround(a, b)  (((a) + ((b) / 2)) / (b))

#define haszero_2(a)    (~(a))
#define haszero_32(a)   (((a) - 0x01010101) & ~(a) & 0x80808080)

/* count population of 1 bits in u32; store into r */
#define onebits_32(u32, r)                                              \
    ((r) = (u32),                                                       \
     (r) -= ((r) >> 1) & 0x55555555,                                    \
     (r) = (((r) >> 2) & 0x33333333) + ((r) & 0x33333333),              \
     (r) = ((((r) >> 4) + (r)) & 0x0f0f0f0f),                           \
     (r) += ((r) >> 8),                                                 \
     (r) += ((r) >> 16),                                                \
     (r) &= 0x3f)
#define onebits_32b(u32, r)                                             \
    ((r) = (u32),                                                       \
     (r) -= ((r) >> 1) & 0x55555555,                                    \
     (r) = (((r) >> 2) & 0x33333333) + ((r) & 0x33333333),              \
     (r) = (((((r) >> 4) + (r)) & 0x0f0f0f0f) * 0x01010101) >> 24)

/* store parity of byte b into r */
#define bytepar(b, r)                                                   \
    do {                                                                \
        unsigned long __tmp1;                                           \
                                                                        \
        __tmp1 = (b);                                                   \
        __tmp1 ^= (b) >> 4;                                             \
        (r) = (0x6996 >> (__tmp1 & 0x0f)) & 0x01;                       \
    } while (0)
#define bytepar2(b, r)                                                  \
    do {                                                                \
        unsigned long __tmp1;                                           \
        unsigned long __tmp2;                                           \
                                                                        \
        __tmp1 = __tmp2 = (b);                                          \
        __tmp2 >>= 4;                                                   \
        __tmp1 ^= __tmp2;                                               \
        __tmp2 = 0x6996;                                                \
        (r) = (__tmp2 >> (__tmp1 & 0x0f)) & 0x01;                       \
    } while (0)
#define bytepar3(b) ((0x6996 >> (((b) ^ ((b) >> 4)) & 0x0f)) & 0x01)

/*
 * round longword u to next power of two if not power of two;
 * store result in r.
 */
#define ceilpow2_32(u, r)                                               \
    do {                                                                \
        (r) = (u);                                                      \
                                                                        \
        if (!powerof2(r)) {                                             \
            (r)--;                                                      \
            (r) |= (r) >> 1;                                            \
            (r) |= (r) >> 2;                                            \
            (r) |= (r) >> 4;                                            \
            (r) |= (r) >> 8;                                            \
            (r) |= (r) >> 16;                                           \
            (r)++;                                                      \
        }                                                               \
    } while (0)
#define ceilpow2_64(u, r)                                               \
    do {                                                                \
        (r) = (u);                                                      \
                                                                        \
        if (!powerof2(r)) {                                             \
            (r)--;                                                      \
            (r) |= (r) >> 1;                                            \
            (r) |= (r) >> 2;                                            \
            (r) |= (r) >> 4;                                            \
            (r) |= (r) >> 8;                                            \
            (r) |= (r) >> 16;                                           \
            (r) |= (r) >> 32;                                           \
            (r)++;                                                      \
        }                                                               \
    } while (0)

/* count number of trailing (low) zero-bits in long-word */
#if defined(__GCC__)
#define tzerol(u) (__builtin_ctzl(u)
#elif defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
#define tzerol(u) (m_scanlo1bit(u))
#elif (LONGSIZE == 4)
static __inline__ unsigned long
tzerol(unsigned long ul)
{
    unsigned long ret;

    tzero32_2(ul, ret);

    return ret;
}
#elif (LONGSIZE == 8)
static __inline__ unsigned long
tzerol(unsigned long ul)
{
    unsigned long ret;

    tzero64_2(ul, ret);

    return ret;
}
#endif

/* count number of leading (high) zero-bits in long-word */
#if defined(__GCC__)
#define lzerol(u) (__builtin_clzl(u));
#elif defined(__i386__) || defined(__x86_64__) || defined(__amd64__)
#define lzerol(u) ((1UL << (LONGSIZELOG2 + 3)) - m_scanhi1bit(u))
#elif (LONGSIZE == 4)
static __inline__ unsigned long
lzerol(unsigned long ul)
{
    unsigned long ret;

    lzero32_2(ul, ret);

    return ret;
}
#elif (LONGSIZE == 8)
static __inline__ unsigned long
lzerol(unsigned long ul)
{
    unsigned long ret;

    lzero64_2(ul, ret);

    return ret;
}
#endif

/* count number of trailing zero-bits in u32 */
#define tzero32_2(u32, r)                                               \
    do {                                                                \
        uint32_t __tmp;                                                 \
        uint32_t __mask;                                                \
                                                                        \
        (r) ^= (r);                                                     \
        __tmp = (u32);                                                  \
        __mask = 0x01;                                                  \
        if (!(__tmp & __mask)) {                                        \
            __mask = 0xffff;                                            \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 16;                                           \
                (r) += 16;                                              \
            }                                                           \
            __mask >>= 8;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 8;                                            \
                (r) += 8;                                               \
            }                                                           \
            __mask >>= 4;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 4;                                            \
                (r) += 4;                                               \
            }                                                           \
            __mask >>= 2;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 2;                                            \
                (r) += 2;                                               \
            }                                                           \
            __mask >>= 1;                                               \
            if (!(__tmp & __mask)) {                                    \
                (r) += 1;                                               \
            }                                                           \
        }                                                               \
    } while (0)

#define lzero32_2(u32, r)                                               \
    do {                                                                \
        uint32_t __tmp;                                                 \
        uint32_t __mask;                                                \
                                                                        \
        (r) ^= (r);                                                     \
        __tmp = (u32);                                                  \
        __mask = 0x01;                                                  \
        __mask <<= CHAR_BIT * sizeof(uint32_t) - 1;                     \
        if (!(__tmp & __mask)) {                                        \
            __mask = 0xffffffff;                                        \
            __mask <<= 16;                                              \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 16;                                           \
                (r) += 16;                                              \
            }                                                           \
            __mask <<= 8;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 8;                                            \
                (r) += 8;                                               \
            }                                                           \
            __mask <<= 4;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 4;                                            \
                (r) += 4;                                               \
            }                                                           \
            __mask <<= 2;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 2;                                            \
                (r) += 2;                                               \
            }                                                           \
            __mask <<= 1;                                               \
            if (!(__tmp & __mask)) {                                    \
                (r)++;                                                  \
            }                                                           \
        }                                                               \
    } while (0)

/* 64-bit versions */

#define tzero64_2(u64, r)                                               \
    do {                                                                \
        uint64_t __tmp;                                                 \
        uint64_t __mask;                                                \
                                                                        \
        (r) = 0;                                                        \
        __tmp = (u64);                                                  \
        __mask = 0x01;                                                  \
        if (!(__tmp & __mask)) {                                        \
            __mask = 0xffffffff;                                        \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 32;                                           \
                (r) += 32;                                              \
            }                                                           \
            __mask >>= 16;                                              \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 16;                                           \
                (r) += 16;                                              \
            }                                                           \
            __mask >>= 8;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 8;                                            \
                (r) += 8;                                               \
            }                                                           \
            __mask >>= 4;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 4;                                            \
                (r) += 4;                                               \
            }                                                           \
            __mask >>= 2;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp >>= 2;                                            \
                (r) += 2;                                               \
            }                                                           \
            __mask >>= 1;                                               \
            if (!(__tmp & __mask)) {                                    \
                (r) += 1;                                               \
            }                                                           \
        }                                                               \
    } while (0)

#define lzero64_2(u64, r)                                               \
    do {                                                                \
        uint64_t __tmp;                                                 \
        uint64_t __mask;                                                \
                                                                        \
        (r) ^= (r);                                                     \
        __tmp = (u64);                                                  \
        __mask = 0x01;                                                  \
        __mask <<= CHAR_BIT * sizeof(uint64_t) - 1;                     \
        if (!(__tmp & __mask)) {                                        \
            __mask = 0xffffffff;                                        \
            __mask <<= 32;                                              \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 32;                                           \
                (r) += 32;                                              \
            }                                                           \
            __mask <<= 16;                                              \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 16;                                           \
                (r) += 16;                                              \
            }                                                           \
            __mask <<= 8;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 8;                                            \
                (r) += 8;                                               \
            }                                                           \
            __mask <<= 4;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 4;                                            \
                (r) += 4;                                               \
            }                                                           \
            __mask <<= 2;                                               \
            if (!(__tmp & __mask)) {                                    \
                __tmp <<= 2;                                            \
                (r) += 2;                                               \
            }                                                           \
            __mask <<= 1;                                               \
            if (!(__tmp & __mask)) {                                    \
                (r)++;                                                  \
            }                                                           \
        }                                                               \
    } while (0)


/* internal macros. */
#define _ftoi32(f)     (*((int32_t *)&(f)))
#define _ftou32(f)     (*((uint32_t *)&(f)))
#define _dtoi64(d)     (*((int64_t *)&(d)))
#define _dtou64(d)     (*((uint64_t *)&(d)))
/* FIXME: little-endian. */
#define _dtohi32(d)    (*(((uint32_t *)&(d)) + 1))
/*
 * IEEE 32-bit
 * 0..22  - mantissa
 * 23..30 - exponent
 * 31     - sign
 */
/* convert elements of float to integer. */
#define fgetmant(f)       (_ftou32(f) & 0x007fffff)
#define fgetexp(f)        ((_ftou32(f) >> 23) & 0xff)
#define fgetsign(f)       (_ftou32(f) >> 31)
#define fsetmant(f, mant) (_ftou32(f) |= (mant) & 0x007fffff)
#define fsetexp(f, exp)   (_ftou32(f) |= ((exp) & 0xff) << 23)
#define fsetsign(f)       (_ftou32(f) | 0x80000000)
/*
 * IEEE 64-bit
 * 0..51  - mantissa
 * 52..62 - exponent
 * 63     - sign
 */
/* convert elements of double to integer. */
#define dgetmant(d)       (_dtou64(d) & UINT64_C(0x000fffffffffffff))
#define dgetexp(d)        ((_dtohi32(d) >> 20) & 0x7ff)
#define dgetsign(d)       (_dtohi32(d) >> 31)
#define dsetmant(d, mant)                                               \
    (*((uint64_t *)&(d)) |= (uint64_t)(mant) | UINT64_C(0x000fffffffffffff))
#define dsetexp(d, exp)                                                 \
    (*((uint64_t *)&(d)) |= (((uint64_t)((exp) & 0x7ff)) << 52))
#define dsetsign(d)                                                     \
    (*((uint64_t *)&(d)) |= UINT64_C(0x8000000000000000))

/*
 * IEEE 80-bit
 * 0..63  - mantissa
 * 64..78 - exponent
 * 79     - sign
 */
#define ldgetmant(ld)       (*((uint64_t *)&ld))
#define ldgetexp(ld)        (*((uint32_t *)&ld + 2) & 0x7fff)
#define ldgetsign(ld)       (*((uint32_t *)&ld + 3) & 0x8000)
#define ldsetmant(ld, mant) (*((uint64_t *)&ld = (mant)))
#define ldsetexp(ld, exp)   (*((uint32_t *)&ld + 2) |= (exp) & 0x7fff)
#define ldsetsign(ld)       (*((uint32_t *)&ld + 3) |= 0x80000000)

/* sign bit 0x8000000000000000. */
#define ifabs(d)                                                        \
    (_dtou64(d) & UINT64_C(0x7fffffffffffffff))
#define fabs2(d, t64)                                                   \
    (*((uint64_t *)&(t64)) = ifabs(d))
/* sign bit 0x80000000. */
#define ifabsf(f)                                                       \
    (_ftou32(f) & 0x7fffffff)

/* TODO: test the stuff below. */

/* (a < b) ? v1 : v2; */
#define condltset(a, b, v1, v2)                                         \
    (((((a) - (b)) >> (CHAR_BIT * sizeof(a) - 1)) & ((v1) ^ (v2))) ^ (v2))

/* c - conditional, f - flag, u - word */
#define condflgset(c, f, u) ((u) ^ ((-(u) ^ (u)) & (f)))

#define satu8(x)                                                        \
    ((x) <= 0xff ? (x) : 0xff)
#define satu16(x)                                                       \
    ((x) <= 0xffff ? (x) : 0xffff)
#define satu32(x)                                                       \
    ((x) <= 0xffffffff ? (x) : 0xffffffff)
#if 0
#define satu8(x16)                                                      \
    ((x16) | (!((x16) >> 8) - 1))
#define satu16(x32)                                                     \
    ((x32) | (!((x32) >> 16) - 1))
#define satu32(x64)                                                     \
    ((x64) | (!((x64) >> 32) - 1))
#define sat8b(x)                                                        \
    condset(x, 0xff, x, 0xff)
#endif

#define haszero(a) (~(a))
#if 0
#define haszero_32(a)                                                   \
    (~(((((a) & 0x7f7f7f7f) + 0x7f7f7f7f) | (a)) | 0x7f7f7f7f))
#endif

/* calculate modulus u % 10 */
#define modu10(u)                                                       \
    ((u) - ((((u) * 6554U) >> 16) * 10))

static __inline__ unsigned long
divu100(unsigned long x)
{
    unsigned long q;
    unsigned long r;

    q = (x >> 1) + (x >> 3) + (x >> 6) - (x >> 10)
        + (x >> 12) + (x >> 13) - (x >> 16);
    q = q + (q >> 20);
    q = q >> 6;
    r = x - q * 100;

    return q + ((r + 28) >> 7);
}

#define modu100(u) ((u) - divu100(u) * 100)
#define modu400(u) ((u) - (divu100(u) >> 2) * 400)

#define leapyear(u)                                                     \
    (!((u) & 0x03) && ((modu100(u)) || !modu400(u)))
#define leapyear2(u)                                                    \
    (!((u) & 0x03) && ((((u) % 100)) || !((u) % 400)))

#endif /* __ZERO_TRIX_H__ */

