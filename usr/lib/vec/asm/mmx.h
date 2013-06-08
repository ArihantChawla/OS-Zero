#ifndef __VEC_MMX_H__
#define __VEC_MMX_H__

#include <zero/param.h>
#include <vec/conf.h>

#if (VECMMX)

#include <mmintrin.h>

/* add 8 signed 8-bit words at adr1 to the ones at adr2 with saturation */
#define vecaddbs_mmx(adr1, adr2, nw)                                    \
    do {                                                                \
        int   *_ptr1 = (int *)(adr1);                                   \
        int   *_ptr2 = (int *)(adr2);                                   \
        int   _i1;                                                      \
        int   _i2;                                                      \
        int   _i3;                                                      \
        int   _i4;                                                      \
        __m64 _zero = _mm_cvtsi32_si64(0);;                             \
        __m64 _word1;                                                   \
        __m64 _word2;                                                   \
        __m64 _tmp1;                                                    \
        __m64 _tmp2;                                                    \
        __m64 _tmp3;                                                    \
        __m64 _res;                                                     \
                                                                        \
        _mm_empty();                                                    \
        while (nw--) {                                                  \
            _i1 = _ptr1[0];                                             \
            _i2 = _ptr2[0];                                             \
            _i3 = _ptr1[1];                                             \
            _i4 = _ptr2[1];                                             \
            _tmp1 = _mm_cvtsi32_si64(_i1);                              \
            _tmp2 = _mm_cvtsi32_si64(_i2);                              \
            _word1 = _mm_unpacklo_pi8(_tmp1, _zero);                    \
            _word2 = _mm_unpacklo_pi8(_tmp2, _zero);                    \
            _tmp3 = _mm_add_pi8(_word1, _word2);                        \
            _res = _mm_packs_pi16(_tmp3, _zero);                        \
            _ptr2[0] = _mm_cvtsi64_si32(_res);                          \
            _tmp1 = _mm_cvtsi32_si64(_i3);                              \
            _tmp2 = _mm_cvtsi32_si64(_i4);                              \
            _word1 = _mm_unpacklo_pi8(_tmp1, _zero);                    \
            _word2 = _mm_unpacklo_pi8(_tmp2, _zero);                    \
            _tmp3 = _mm_add_pi8(_word1, _word2);                        \
            _res = _mm_packs_pi16(_tmp3, _zero);                        \
            _ptr2[1] = _mm_cvtsi64_si32(_res);                          \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)

#define vecaddbu_mmx(adr1, adr2, nw)                                    \
    do {                                                                \
        int   *_ptr1 = (int *)(adr1);                                   \
        int   *_ptr2 = (int *)(adr2);                                   \
        int   _i1;                                                      \
        int   _i2;                                                      \
        int   _i3;                                                      \
        int   _i4;                                                      \
        int   _n = (nw);                                                \
        __m64 _zero = _mm_cvtsi32_si64(0);;                             \
        __m64 _word1;                                                   \
        __m64 _word2;                                                   \
        __m64 _tmp1;                                                    \
        __m64 _tmp2;                                                    \
        __m64 _tmp3;                                                    \
        __m64 _res;                                                     \
                                                                        \
        _mm_empty();                                                    \
        while (_n--) {                                                  \
            _i1 = _ptr1[0];                                             \
            _i2 = _ptr2[0];                                             \
            _i3 = _ptr1[1];                                             \
            _i4 = _ptr2[1];                                             \
            _tmp1 = _mm_cvtsi32_si64(_i1);                              \
            _tmp2 = _mm_cvtsi32_si64(_i2);                              \
            _word1 = _mm_unpacklo_pi8(_tmp1, _zero);                    \
            _word2 = _mm_unpacklo_pi8(_tmp2, _zero);                    \
            _tmp3 = _mm_add_pi8(_word1, _word2);                        \
            _res = _mm_packs_pu16(_tmp3, _zero);                        \
            _ptr2[0] = _mm_cvtsi64_si32(_res);                          \
            _tmp1 = _mm_cvtsi32_si64(_i3);                              \
            _tmp2 = _mm_cvtsi32_si64(_i4);                              \
            _word1 = _mm_unpacklo_pi8(_tmp1, _zero);                    \
            _word2 = _mm_unpacklo_pi8(_tmp2, _zero);                    \
            _tmp3 = _mm_add_pi8(_word1, _word2);                        \
            _res = _mm_packs_pu16(_tmp3, _zero);                        \
            _ptr2[1] = _mm_cvtsi64_si32(_res);                          \
            _ptr1 += 2;                                                 \
            _ptr2 += 2;                                                 \
        }                                                               \
    } while (0)

#endif /* VECMMX */

#endif /* __VEC_MMX_H__ */

