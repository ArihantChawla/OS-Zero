/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0376c/Babbfeda.html */
/* http://www.arch.eece.maine.edu/ece473/images/4/4a/Ece473_hw3_IEEE754.pdf */
#ifndef __BITS_IEEE754_H__
#define __BITS_IEEE754_H__

#include <stdint.h>

/* characteristics of float */
#define FLT_DIG         6
#define FLT_MANT_DIG    24
#define FLT_MIN_EXP     (-125)
#define FLT_MAX_EXP     128
#define FLT_MIN_10_EXP  (-37)
#define FLT_MAX_10_EXP  38
#define FLT_DECIMAL_DIG 9

/* characteristics of double */
#define DBL_DIG         15
#define DBL_MANT_DIG    53
#define DBL_MIN_EXP     (-1021)
#define DBL_MAX_EXP     1024
#define DBL_MIN_10_EXP  (-307)
#define DBL_MAX_10_EXP  308
#define DBL_DECIMAL_DIG 17

/* LDBL-macros are defined in architecture header files, e.g. <ia32/float.h> */

#define fpclassify(x)                                                   \
    (((sizeof(x) == sizeof(double))                                     \
      ? __fpclassify(x)                                                 \
      : (((sizeof(x) == sizeof(float))                                  \
          ? __fpclassifyf(x)                                            \
          : __fpclassifyl(x)))))

/*
 * isnanl() and __isqnanl() are defined in a per-architecture header such as
 * <x86-64/math.h>
 */
/* NOTE: isinf(), isnan(), finite() are from BSD */
#define __isinf(x)                                                      \
    (!dgetmant(x)                                                       \
     && ((*(uint64_t *)&(x) & UINT64_C(0x7ff0000000000000))             \
         == UINT64_C(0x7ff0000000000000)))
#define __isinff(x)                                                     \
    (!fgetmant(x) && ((*(uint32_t *)&(x) & 0x7ff00000) == 0x7ff00000))
#define __isnan(x)                                                      \
    ((*(uint64_t *)&(x) & UINT64_C(0x7fffffffffffffff))                 \
     == UINT64_C(0x7fffffffffffffff))
#define __isnanf(x)                                                     \
    ((*(uint32_t *)&(x) & 0x7fffffff) == 0x7fffffff)
#define __issignan(x)                                                   \
    (!(*(uint64_t *)&(x) & UINT64_C & UINT64_C(0x0008000000000000)))
#define __issignanf(x)                                                  \
    (!(*(uint32_t *)&(x) & 0x00400000))

#define isfinite(x)                                                     \
    (((sizeof(x) == sizeof(double))                                     \
      ? __isfinite(x)                                                   \
      : (((sizeof(x) == sizeof(float))                                  \
          ? __isfinitef(x)                                              \
          : __isfinitel(x)))))
#define isnormal(x)                                                     \
    (((sizeof(x) == sizeof(double))                                     \
      ? (__fpclassify(x) == FP_NORMAL)                                  \
      : (((sizeof(x) == sizeof(float))                                  \
          ? ( __fpclassifyf(x) == FP_NORMAL)                            \
          : (__fpclassifyl(x) == FP_NORMAL)))))
#define issignaling(x)                                                  \
    (((sizeof(x) == sizeof(double))                                     \
      ? (__isnan(x) && __issignan(x))                                   \
      : (((sizeof(x) == sizeof(float))                                  \
          ? (__isnanf(x) & __issignanf(x))                              \
          (__isnanl(x) && __issignanl(x))))))

#if defined(USEBSD) && (USEBSD)
#define isinf(x)   __isinf(x)
#define isinff(x)  __isinff(x)
#define isinfl(x)  __isinfl(x)
#define isnan(x)   __isnan(x)
#define isnanf(x)  __isnanf(x)
#define isnanl(x)  __isnanl(x)
#define finite(x)  (!__isinf(x))
#define finitef(x) (!__isinff(x))
#define finitel(x) (!__isinfl(x))
#endif /* USEBSD */

#endif /* __BITS_IEEE754_H__ */

