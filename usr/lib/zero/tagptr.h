#ifndef __ZERO_TAGPTR_H__
#define __ZERO_TAGPTR_H__

/* tagged pointers for libzero */

#include <stdint.h>

#define tagptrcmpswap(tp, want, src)                                    \
    m_cmpswapdbl((volatile long *)tp,                                   \
                 (volatile long *)want,                                 \
                 (volatile long *)src)

#if defined(__x86_64__) || defined(__amd64__)

#include <xmmintrin.h>
#include <emmintrin.h>

#define TAGPTR_T __m128i

#define tagptrinitadr(adr, tp)                                          \
    ((tp) = _mm_cvtsi64_si128((long long)adr), (tp) = _mm_slli_si128((tp), 64))
#define tagptrcpytag(src, dest)                                         \
    ((dest) = _mm_or_si128(dest, _mm_loadl_epi64(&src)))
#define tagptrinctag(tp)                                                \
    (_mm_add_ss((__m128)tp, (__m128)_mm_cvtsi64_si128(INT64_C(1))))
#define tagptrgetadr(tp)                                                \
    ((void *)((uintptr_t)_mm_cvtsi128_si64(_mm_srli_si128(tp, 64))))

struct tagptr {
    union {
        TAGPTR_T     _aln;      // force dualword-alignment
        struct {
            uint64_t  tag;      // tag in low word to allow easy increment
            void     *adr;      // pointer value
        } ptr;
    } data;
};

#endif

#endif /* __ZERO_TAGPTR_H__ */

