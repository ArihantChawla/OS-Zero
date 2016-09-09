#ifndef __ZERO_X86_64_TAGPTR_H__
#define __ZERO_X86_64_TAGPTR_H__

#include <stddef.h>
//#include <xmmintrin.h>
#include <emmintrin.h>
#include <zero/asm.h>
//#include <zero/op128.h>
#include <zero/x86-64/bignum.h>

#if defined(__SIZEOF_INT128__)
#define TAGPTR_T     op128_t
#else
/* pointer in the high-order 64 bits, tag [counter] in the low */
#define TAGPTR_T     __m128i
#endif
#define TAGPTR_ADR_T uint64_t
#define TAGPTR_TAG_T uint64_t

struct _tagptr {
    uint64_t tag;
    uint64_t adr;
};

#define tagptrcmpswap(tp, want, src)                                    \
m_cmpswapdbl((volatile long *)tp, (long *)want, (long *)src)

#if defined(__SIZEOF_INT128__)
#define tagptrzero(tp) (*(tp) = (TAGPTR_T)0)
/* tp.adr = adr */
#define tagptrsetadr(adr, tp)                                           \
do {                                                                    \
    TAGPTR_T _tp;                                                       \
    TAGPTR_T _tag;                                                      \
                                                                        \
    _tp = (TAGPTR_ADR_T)(adr);                                          \
    _tag = (tp);                                                        \
    _tp <<= 64;                                                         \
    _tag &= (TAGPTR_T)(~UINT64_C(0));                                   \
    _tp |= _tag;                                                        \
    (tp) = _tp;                                                         \
} while (0)
/* adr = tp.adr */
#define tagptrgetadr(tp, adr)                                           \
    do {                                                                \
      TAGPTR_T _adr;                                                    \
                                                                        \
      _adr = (tp);                                                      \
      _adr >>= 64;                                                      \
      (adr) = (void *)(TAGPTR_ADR_T)_adr;                               \
  } while (0)
/* tag = tp.tag */
#define tagptrgettag(tp, tag)                                           \
    ((tag) = (TAGPTR_TAG_T)(tp) & (~UINT64_C(0)))
/* tp.tag = tag */
#define tagptrsettag(tag, tp)                                           \
  do {                                                                  \
      TAGPTR_T _tp;                                                     \
      TAGPTR_T _tag;                                                    \
                                                                        \
      _tp = (tp);                                                       \
      _tag = (tag);                                                     \
      _tp &= (TAGPTR_T)(~UINT64_C(0));                                  \
      _tp <<= 64;                                                       \
      _tp |= _tag;                                                      \
      (tp) = _tp;                                                       \
  } while (0)
#define tagptrcmp(tp1, tp2) ((tp1) == (tp2))
#else
#define tagptrzero(tp) opstzero128(tp)
/* tp.adr = adr */
#define tagptrsetadr(adr, tp)                                           \
    do {                                                                \
      TAGPTR_T _tagmask;                                                \
                                                                        \
      _tagmask = _mm_cvtsi64_si128(~UINT64_C(0));                       \
      (tp) = _mm_and_si128(tp, _tagmask);                               \
      (tp) = _mm_or_si128(tp,                                           \
                          _mm_slli_si128(_mm_cvtsi64_si128((long long)adr), \
                                         8));                           \
  } while (0)
/* adr = tp.adr */
#define tagptrgetadr(tp, adr)                                           \
    do {                                                                \
      uintptr_t _upval = _mm_cvtsi128_si64(_mm_srli_si128(tp, 8));      \
                                                                        \
      (adr) = (void *)_upval;                                           \
  } while (0)
/* tag = tp.tag */
#define tagptrgettag(tp, tag)                                           \
    do {                                                                \
      (tag) = _mm_cvtsi128_si64(tp);                                    \
  } while (0)
/* tp.tag = tag */
#define tagptrsettag(tag, tp)                                           \
    do {                                                                \
      TAGPTR_T _adrmask;                                                \
                                                                        \
      _adrmask = _mm_slli_si128(_mm_cvtsi64_si128(~UINT64_C(0)), 8);    \
      (tp) = _mm_and_si128((tp), _adrmask);                             \
      (tp) = _mm_or_si128((tp), _mm_cvtsi64_si128(tag));                \
  } while (0)
static __inline__ long
tagptrcmp(TAGPTR_T *tp1, TAGPTR_T *tp2)
{
    i128 diff = sub128(*((i128 *)tp1), *((i128 *)tp2));

    return (diff == 0);
}
#endif

#endif /* __ZERO_X86_64_TAGPTR_H__ */

