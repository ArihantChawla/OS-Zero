#ifndef __ZERO_BITS_MEM_H__
#define __ZERO_BITS_MEM_H__

#include <stdint.h>
#include <mach/param.h>
#include <zero/cdefs.h>
#include <zero/trix.h>
#include <zero/randklc.h>

/* minimum allocation block size */
#define MEMMINALIGN    CLSIZE
#define MEMALIGNSHIFT  CLSIZELOG2
#define MEM_MIN_SIZE   MEMMINALIGN

/* custom allocator types */
/* size of machine word/integer */
#if (WORDSIZE == 8)
#define MEMWORD_T      int64_t
#define MEMUWORD_T     uint64_t
#elif (WORDSIZE == 4)
#define MEMWORD_T      int32_t
#define MEMUWORD_T     uint32_t
#else
#error WORDSIZE not supported in <zero/bits/mem.h>
#endif
/* byte-pointer for pointer-arithmetics */
#define MEMPTR_T       uint8_t *
/* integer type for pointer values */
#define MEMADR_T       uintptr_t
/* buffer block ID type */
#define MEMBLK_T       int16_t

/* argument flags for queue functions */
#define MEMQUEUEGLOBAL (1L << 0)
/* failure code for lock-routines */
#define MEMLKFAIL        ((void *)(~((MEMADR_T)0)))
/* lock-bit values */
#define MEM_ADR_LK_BIT_POS 0
#define MEM_ADR_LK_BIT     ((uintptr_t)1 << MEM_ADR_LK_BIT_POS)

/* align pointer to aln-byte boundary */
#define memalignptr(ptr, aln)   ((void *)((uintptr_t)(ptr) & ~((align) - 1)))
#define memptraligned(ptr, aln) (!(MEMADR_T(ptr) & ((aln) - 1)))

/* simple randomization of block addresses for cache-coloring */
#define memcoloradr(adr, aln, mul)                                      \
    ((void *)((uint8_t *)(adr) + (mul) * (memrandofs() & ((aln) - 1))))
#if (PTRSIZE == 8)
#define memrandofs() i_randklc64(res)
#elif (PTRSIZE == 4)
#define memrandofs() i_randklc32(res)
#else
#error PTRSIZE not supported in <zero/bits/mem.h>
#endif

/* calculate bucket for small allocation of sz bytes; multiple of CLSIZE */
#define _memcalcblkpool(sz, pool)                                       \
    do {                                                                \
        MEMUWORD_T _res = sz;                                           \
                                                                        \
        _res--;                                                         \
        _res >>= CLSIZELOG2;                                            \
        (pool) = _res;                                                  \
    } while (0)
/* calculate bucket for page-run of sz bytes; multiple of PAGESIZE */
#define _memcalcrunpool(sz, pool)                                       \
    do {                                                                \
        MEMUWORD_T _res = sz;                                           \
                                                                        \
        _res--;                                                         \
        _res >>= PAGESIZELOG2;                                          \
        (pool) = _res;                                                  \
    } while (0)
/* calculate bucket for mid-size run of sz bytes; multiple of 8 * PAGESIZE */
#define _memcalcmidpool(sz, pool)                                       \
    do {                                                                \
        MEMUWORD_T _res = sz;                                           \
                                                                        \
        _res--;                                                         \
        _res >>= MEM_MID_UNIT_SHIFT + PAGESIZELOG2;                     \
        (pool) = _res;                                                  \
    } while (0)
/* calculate bucket for big global allocation; a power of two */
#if (WORDSIZE == 4)
#define _memcalcbigpool(sz, pool)                                       \
    do {                                                                \
        int32_t _tmp;                                                   \
        int32_t _pool;                                                  \
                                                                        \
        ceilpow2_32(sz, _tmp);                                          \
        _pool = tzerol(_tmp);                                           \
        (pool) = _pool;                                                 \
    } while (0)
#elif (WORDSIZE == 8)
#define _memcalcbigpool(sz, pool)                                       \
    do {                                                                \
        int64_t _tmp;                                                   \
        int64_t _pool;                                                  \
                                                                        \
        ceilpow2_64(sz, _tmp);                                          \
        _pool = tzeroll(_tmp);                                          \
        (pool) = _pool;                                                 \
    } while (0)
#endif

/* generate an address in the range ptr + [0, 8 * CLSIZE] */
static __inline__ MEMPTR_T
memgenadr(MEMPTR_T ptr)
{
    MEMPTR_T   adr = ptr;
    MEMADR_T   res = (MEMADR_T)ptr;
//    MEMWORD_T lim = blksz - size;
    MEMWORD_T  shift;
    MEMADR_T   q;
    MEMADR_T   r;
    MEMADR_T   div9;
    MEMADR_T   dec;

#if (CLSIZE == 32)
    shift = 3;
#elif (CLSIZE == 64)
    shift = 4;
#endif
    /* shift out some [mostly-aligned] low bits */
    res >>= MEMALIGNSHIFT;
    /* divide by 9 */
    q = res - (res >> 3);
    q = q + (q >> 6);
    q = q + (q >> 12) + (q >> 24);
    q = q >> 3;
    r = res - q * 9;
    div9 = q + ((r + 7) >> 4);
    /* calculate res -= res/9 * 9 i.e. res % 9 (max 8) */
    dec = div9 * 9;
    res -= dec;
    /* scale by shifting the result of the range 0..8 */
    res <<= shift;
    /* round down to a multiple of cacheline */
    res &= ~(CLSIZE - 1);
    /* add offset to original address */
    adr += res;

    return adr;
}

/* generate an offset in the range [0, 4 * CLSIZE] */
static __inline__ MEMADR_T
memgenofs(MEMPTR_T ptr)
{
    MEMADR_T  res = (MEMADR_T)ptr;
    MEMWORD_T shift;
    MEMADR_T  q;
    MEMADR_T  r;
    MEMADR_T  div9;
    MEMADR_T  dec;

#if (CLSIZE == 32)
    shift = 4;
#elif (CLSIZE == 64)
    shift = 5;
#endif
    /* shift out some [mostly-aligned] low bits */
    res >>= MEMALIGNSHIFT;
    /* divide by 9 */
    q = res - (res >> 3);
    q = q + (q >> 6);
    q = q + (q >> 12) + (q >> 24);
    q = q >> 3;
    r = res - q * 9;
    div9 = q + ((r + 7) >> 4);
    /* calculate res -= res/9 * 9 i.e. res % 9 (max 8) */
    dec = div9 * 9;
    res -= dec;
    /* scale by shifting the result of the range 0..8 */
    res <<= shift;
    /* round down to a multiple of cacheline */
    res &= ~(CLSIZE - 1);
    /* add offset to original address */

    return res;
}

/* compute adr + adr % 9 (# of words in offset, aligned to word boundary) */
static __inline__ MEMWORD_T *
memgentabadr(MEMWORD_T *adr)
{
    MEMADR_T res = (MEMADR_T)adr;
    MEMADR_T q;
    MEMADR_T r;
    MEMADR_T div9;
    MEMADR_T dec;

    /* shift out some [mostly-aligned] low bits */
    res >>= 16;
    /* divide by 9 */
    q = res - (res >> 3);
    q = q + (q >> 6);
    q = q + (q >> 12) + (q >> 24);
    q = q >> 3;
    r = res - q * 9;
    div9 = q + ((r + 7) >> 4);
    /* calculate res -= res/9 * 9 i.e. res % 9 (max 8) */
    dec = div9 * 9;
    res -= dec;
    /* scale res to 0..32 (machine words) */
    res <<= 2;
    /* add to original pointer */
    adr += res;
    /* align to machine word boundary */

    return adr;
}

#endif /* __ZERO_BITS_MEM_H__ */

