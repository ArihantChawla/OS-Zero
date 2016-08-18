#ifndef __ZERO_X86_64_TAGPTR_H__
#define __ZERO_X86_64_TAGPTR_H__

#include <stddef.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <zero/asm.h>
#include <zero/x86-64/bignum.h>

/* pointer in the high-order 64 bits, tag [counter] in the low */
#define TAGPTR_T     __m128i
#define TAGPTR_ADR_T void *
#define TAGPTR_TAG_T uint64_t

#define tagptrcmpswap(tp, want, src)                                    \
    m_cmpswapdbl((volatile long *)tp,                                   \
                 (volatile long *)want,                                 \
                 (volatile long *)src)

/* tp = { 0, adr } */
#if 0
#define tagptrinitadr(adr, tp)                                          \
    do {                                                                \
        TAGPTR_T _tptr = _mm_cvtsi64_si128((long long)adr);             \
                                                                        \
        _tptr = _mm_slli_si128(_tptr, 64);                              \
        (tp) = _tptr;                                                   \
    } while (0)
#endif
#define tagptrinitadr(adr, tp)                                          \
    ((tp) = _mm_cvtsi64_si128((long long)adr), (tp) = _mm_slli_si128((tp), 64))
#if 0
#define tagptrinitadr(adr, tp)                                          \
    ((tp) = _mm_slli_si128(_mm_cvtsi64_si128(adr), 64))
#endif
/* dest.tag = src.tag */
#define tagptrgettag(tp)                                                \
    _mm_cvtsi128_si64(tp)
#define tagptrsettag(tag, tp)                                           \
    ((tp) = _mm_or_si128(tp, _mm_cvtsi64_si128(tag)))
/* tag.adr */
#define tagptrgetadr(tp)                                                \
    ((void *)((uintptr_t)_mm_cvtsi128_si64(_mm_srli_si128(tp, 64))))

static __inline__ long
tagptrcmp(TAGPTR_T *tp1, TAGPTR_T *tp2)
{
    i128 diff = sub128(*((i128 *)tp1), *((i128 *)tp2));

    return (diff == 0);
}

#endif /* __ZERO_X86_64_TAGPTR_H__ */

