/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0376c/Babbfeda.html */
/* http://www.arch.eece.maine.edu/ece473/images/4/4a/Ece473_hw3_IEEE754.pdf */
#ifndef __BITS_IEEE754_H__
#define __BITS_IEEE754_H__

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
#define isfinite(x)                                                     \
    (((sizeof(x) == sizeof(double))                                     \
      ? __isfinite(x)                                                   \
      : (((sizeof(x) == sizeof(float))                                  \
          ? __isfinitef(x)                                              \
          : __isfinitel(x)))))

#endif /* __BITS_IEEE754_H__ */

