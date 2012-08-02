#ifndef __ZERO_TRIX_H__
#define __ZERO_TRIX_H__

#include <features.h>

/*
 * this file contains tricks I've gathered together from sources such as MIT
 * HAKMEM and the book Hacker's Delight
 */

#define ZEROABS 1

#include <stdint.h>
#include <limits.h>
#include <zero/param.h>

#if (LONGSIZE == 4)
#define tzerol(u, r) tzero32(u, r)
#define lzerol(u, r) lzero32(u, r)
#elif (LONGSIZE == 8)
#define tzerol(u, r) tzero64(u, r)
#define lzerol(u, r) lzero64(u, r)
#endif

/* get the lowest 1-bit in a */
#define lo1bit(a)           ((a) & -(a))
/* get n lowest and highest bits of i */
#define lobits(i, n)        ((i) & ((1U << (n)) - 0x01))
#define hibits(i, n)        ((i) & ~((1U << (sizeof(i) * CHAR_BIT - (n))) - 0x01))
/* get n bits starting from index j */
#define getbits(i, j, n)    (lobits((i) >> (j), (n)))
#define setbits(i, j, n, b) ((i) |= (((b) << (j)) & ~(((1U << (n)) << (j)) - 0x01)))
#define bitset(p, b)        (((uint8_t *)(p))[(b) >> 3] & (1U << ((b) & 0x07)))
/* set bit # b in *p */
#define setbit(p, b)        (((uint8_t *)(p))[(b) >> 3] |= (1U << ((b) & 0x07)))
/* clear bit # b in *p */
#define clrbit(p, b)        (((uint8_t *)(p))[(b) >> 3] &= ~(1U << ((b) & 0x07)))

/* compute minimum and maximum of a and b without branching */
#define min(a, b)                                                       \
    ((b) + (((a) - (b)) & -((a) < (b))))
#define max(a, b)                                                       \
    ((a) - (((a) - (b)) & -((a) < (b))))
/* compare with power-of-two p2 */
#define gt2(u, p2)  /* true if u > p2 */                                \
    ((u) & ~(p2))
#define gte2(u, p2) /* true if u >= p2 */                               \
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
#define mod2(a, b2)     ((a) & ((b2) - 1))

/* round a up to the next multiple of (the power of two) b2. */
#define roundup1(a, b2) (((a) + ((b2) - 0x01)) & ~((b2) + 0x01))
#define roundup2(a, b2) (((a) + ((b2) - 0x01)) & -(b2))

/* round down to the previous multiple of (the power of two) b2 */
#define rounddown2(a, b2) ((a) & ~((b2) - 0x01))

/* compute the average of a and b without division */
#define uavg(a, b)      (((a) & (b)) + (((a) ^ (b)) >> 1))

#define divceil(a, b)   (((a) + (b) - 1) / (b))
#define divround(a, b)  (((a) + ((b) / 2)) / (b))

#define haszero_2(a)    (~(a))
#define haszero_32(a)   (((a) - 0x01010101) & ~(a) & 0x80808080)

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

#define bytepar(b, r)                                                   \
    do {                                                                \
        unsigned long _tmp1;                                            \
                                                                        \
        _tmp1 = (b);                                                    \
        _tmp1 ^= (b) >> 4;                                              \
        (r) = (0x6996 >> (_tmp1 & 0x0f)) & 0x01;                        \
    } while (0)
#define bytepar2(b, r)                                                  \
    do {                                                                \
        unsigned long _tmp1;                                            \
        unsigned long _tmp2;                                            \
                                                                        \
        _tmp1 = _tmp2 = (b);                                            \
        _tmp2 >>= 4;                                                    \
        _tmp1 ^= _tmp2;                                                 \
        _tmp2 = 0x6996;                                                 \
        (r) = (_tmp2 >> (_tmp1 & 0x0f)) & 0x01;                         \
    } while (0)
#define bytepar3(b) ((0x6996 >> (((b) ^ ((b) >> 4)) & 0x0f)) & 0x01)

/* count number of trailing zero-bits in u32 */
#define tzero32(u32, r)                                                 \
    do {                                                                \
        uint32_t __tmp;                                                 \
        uint32_t __mask;                                                \
                                                                        \
        (r) = 0;                                                        \
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

/*
 * count number of leading zero-bits in u32
 */
#define lzero32(u32, r)                                                 \
    ((u32) |= ((u32) >> 1),                                             \
     (u32) |= ((u32) >> 2),                                             \
     (u32) |= ((u32) >> 4),                                             \
     (u32) |= ((u32) >> 8),                                             \
     (u32) |= ((u32) >> 16),                                            \
     CHAR_BIT * sizeof(u32) - onebits_32(u32, r))

/* 64-bit versions */

#define tzero64(u64, r)                                                 \
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

#define lzero64(u64, r)                                                 \
    do {                                                                \
        uint64_t __tmp;                                                 \
        uint64_t __mask;                                                \
\
        (r) = 0;   \
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
#define dsetmant(d, mant) (*((uint64_t *)&(d)) |= (uint64_t)(mant) | UINT64_C(0x000fffffffffffff))
#define dsetexp(d, exp)   (*((uint64_t *)&(d)) |= (((uint64_t)((exp) & 0x7ff)) << 52))
#define dsetsign(d)       (*((uint64_t *)&(d)) |= UINT64_C(0x8000000000000000))

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
#define condsetf(c, f, u) ((u) ^ ((-(u) ^ (u)) & (f)))

#define nextp2(a)                                                       \
    (((a)                                                               \
      | ((a) >> 1)                                                      \
      | ((a) >> 2)                                                      \
      | ((a) >> 4)                                                      \
      | ((a) >> 8)                                                      \
      | ((a) >> 16)) + 1)

/* (a < b) ? v1 : v2; */
#define condset(a, b, v1, v2)                                           \
    (((((a) - (b)) >> (CHAR_BIT * sizeof(a) - 1)) & ((v1) ^ (v2))) ^ (v2))

/* c - conditional, f - flag, u - word */
#define condsetf(c, f, u) ((u) ^ ((-(u) ^ (u)) & (f)))

#define sat8(x)                                                         \
    ((x) | (!((x) >> 8) - 1))
#define sat8b(x)                                                        \
    condset(x, 0xff, x, 0xff)

/* m - mask of bits to be taken from b. */
#define mergebits(a, b, m)  ((a) ^ (((a) ^ (b)) & (m)))
/* m - mask of bits to be copied from a. 1 -> copy, 0 -> leave alone. */
#define copybits(a, b, m) (((a) | (m)) | ((b) & ~(m)))

#define haszero(a) (~(a))
#if 0
#define haszero_32(a)                                                   \
    (~(((((a) & 0x7f7f7f7f) + 0x7f7f7f7f) | (a)) | 0x7f7f7f7f))
#endif

/* calculate modulus u % 10 */
#define modu10(u)                                                       \
    ((u) - ((((u) * 6554U) >> 16) * 10))

/* TODO: change modulus calculations to something faster */
#define leapyear(x)                                                     \
    (!((x) & 0x03) && (((x) % 100)) || !((x) % 400))

#endif /* __ZERO_TRIX_H__ */

