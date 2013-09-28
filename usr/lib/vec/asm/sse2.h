#ifndef __VEC_SSE2_H__
#define __VEC_SSE2_H__

#include <zero/param.h>
#include <vec/conf.h>
#include <vec/vec.h>
#include <vec/asm/mmx.h>

#if (VECSSE2)

#include <emmintrin.h>

/* add signed 8-bit values, saturate to -0x7f-1..0x7f */
#define vecadd8ss_sse2(adr1, adr2, nw)                                  \
    do {                                                                \
        int      _nleft = (nw) & 0x01;                                  \
        int      _n = (nw) >> 1;                                        \
        __m128i *_src = (__m128i *)adr1;                                \
        __m128i *_dest = (__m128i *)adr2;                               \
        __m128i _word1;                                                 \
        __m128i _word2;                                                 \
                                                                        \
        _mm_empty();                                                    \
        while (_n--) {                                                  \
            _word1 = _mm_load_si128(_src);                              \
            _word2 = _mm_load_si128(_dest);                             \
            _word2 = _mm_adds_epi8(_word1, _word2);                     \
            _mm_store_si128(_dest, _word2);                             \
            _src++;                                                     \
            _dest++;                                                    \
        }                                                               \
        if (_nleft) {                                                   \
            vecadd8ss(_src, _dest, 1);                                  \
        }                                                               \
    } while (0)

/* add unsigned 8-bit values, saturate to 0..0xff */
#define vecadd8us_sse2(adr1, adr2, nw)                                  \
    do {                                                                \
        int      _nleft = (nw) & 0x01;                                  \
        int      _n = (nw) >> 1;                                        \
        __m128i *_src = (__m128i *)adr1;                                \
        __m128i *_dest = (__m128i *)adr2;                               \
        __m128i _word1;                                                 \
        __m128i _word2;                                                 \
                                                                        \
        _mm_empty();                                                    \
        while (_n--) {                                                  \
            _word1 = _mm_load_si128(_src);                              \
            _word2 = _mm_load_si128(_dest);                             \
            _word2 = _mm_adds_epu8(_word1, _word2);                     \
            _mm_store_si128(_dest, _word2);                             \
            _src++;                                                     \
            _dest++;                                                    \
        }                                                               \
        if (_nleft) {                                                   \
            vecadd8us(_src, _dest, 1);                                  \
        }                                                               \
    } while (0)

#endif /* VECSSE2 */

#endif /* __VEC_SSE2_H__ */

