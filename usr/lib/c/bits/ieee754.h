/* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0376c/Babbfeda.html */
#ifndef __BITS_IEEE754_H__
#define __BITS_IEEE754_H__

/* characteristics of float */
#define FLT_MANT_DIG   24
#define FLT_MIN_EXP    (-125)
#define FLT_MAX_EXP    128
#define FLT_MIN        (1.175494351e-38f)
#define FLT_MAX        (3.40282347e+38f)
#define FLT_EPSILON    (1.19209290e-7f)
#define FLT_DIG        6
#define FLT_MIN_10_EXP (-37)
#define FLT_MAX_10_EXP 38

/* characteristics of double */
#define DBL_MANT_DIG   53
#define DBL_MIN_EXP    (-1021)
#define DBL_MAX_EXP    1024
#define DBL_MIN        (2.22507385850720138e-308)
#define DBL_MAX        (1.79769313486231571e+308)
#define DBL_EPSILON    (2.2204460492503131e-16)
#define DBL_DIG        15
#define DBL_MIN_10_EXP (-307)
#define DBL_MAX_10_EXP 308

/* LDBL - define these in <ia32/float.h> (80- and 128-bit) */

#endif /* __BITS_IEEE754_H__ */

