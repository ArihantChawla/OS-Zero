/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0376c/Babbfeda.html */
/* http://www.arch.eece.maine.edu/ece473/images/4/4a/Ece473_hw3_IEEE754.pdf */
#ifndef __BITS_IEEE754_H__
#define __BITS_IEEE754_H__

/* characteristics of float */
#define FLT_DIG        6
#define FLT_MANT_DIG   24
#define FLT_MIN_EXP    (-125)
#define FLT_MAX_EXP    128
#if 0
#define FLT_MIN        (1.175494351E-38F)
#define FLT_MAX        (3.402823466E+38F)
#define FLT_EPSILON    (1.192092896E-7F)
#endif
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_10_EXP 38

/* characteristics of double */
#define DBL_DIG        15
#define DBL_MANT_DIG   53
#define DBL_MIN_EXP    (-1021)
#define DBL_MAX_EXP    1024
#if 0
#define DBL_MIN        (2.2250738585072014E-308)
#define DBL_MAX        (1.7976931348623157E+308)
#define DBL_EPSILON    (2.2204460492503131E-16)
#endif
#define DBL_MIN_10_EXP (-307)
#define DBL_MAX_10_EXP 308

/* LDBL-macros are defined in architecture header files, e.g. <ia32/float.h> */

#endif /* __BITS_IEEE754_H__ */

