#ifndef __ZERO_X86_64_TAGPTR_H__
#define __ZERO_X86_64_TAGPTR_H__

#include <xmmintrin.h>
#include <emmintrin.h>

/* pointer in the high-order 64 bits, tag [counter] in the low */
#define TAGPTR_T __m128i

#define tagptrinitadr(adr, tp)                                          \
    ((tp) = _mm_cvtsi64_si128((long long)adr), (tp) = _mm_slli_si128((tp), 64))
#define tagptrcpytag(src, dest)                                         \
    ((dest) = _mm_or_si128(dest, _mm_loadl_epi64(&src)))
#define tagptrinctag(tp)                                                \
    (_mm_add_ss((__m128)tp, (__m128)_mm_cvtsi64_si128(INT64_C(1))))
#define tagptrgetadr(tp)                                                \
    ((void *)((uintptr_t)_mm_cvtsi128_si64(_mm_srli_si128(tp, 64))))

#endif /* __ZERO_X86_64_TAGPTR_H__ */

