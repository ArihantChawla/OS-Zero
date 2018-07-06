#ifndef __ZERO_BITS_MEM_H__
#define __ZERO_BITS_MEM_H__

#include <stdint.h>
#include <zero/param.h>
#include <zero/cdefs.h>
#include <zero/trix.h>
#include <zero/randklc.h>

/* minimum allocation block size */
#define MEMMINALIGN    CLSIZE
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
#define MEMPTR_T       uint8 *
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
#define memalignptr(ptr, aln)   ((void *)((uintptr)(ptr) & ((align) - 1)))
#define memptraligned(ptr, aln) (!(MEMADR_T(ptr) & ((aln) - 1)))

/* lowest-order bit lock for pointers */
#define memtrylkptr(ptr)                                                \
    (!m_cmpsetbit((m_atomic_t *)(ptr), MEM_ADR_LK_BIT_POS))
static __inline__
memlkptr(m_atomicptr_t *ptr)
{
    do {
        while (*(m_atomic_t *)ptr & MEM_ADR_LK_BIT) {
            m_spinwait();
        }
        if (memtrylkptr(ptr)) {

            return;
        }
    } while (1);
}

/* simple randomization of block addresses for cache-coloring */
#define memgenadr(adr, aln, mul)                                        \
    ((void *)((uint8_t *)(adr) + (mul) * (memrandofs() & ((aln) - 1))))
#if (PTRSIZE == 8)
#define memrandofs() i_randklc64(res)
#elif (PTRSIZE == 4)
#define memrandofs() i_randklc32(res)
#else
#error PTRSIZE not supported in <zero/bits/mem.h>
#endif

/* calculate bucket for small allocation of sz bytes; multiple of CLSIZE */
#define _memcalcsmallpool(sz, pool)                                     \
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

#endif /* __ZERO_BITS_MEM_H__ */

