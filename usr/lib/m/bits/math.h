#ifndef __BITS_MATH_H__
#define __BITS_MATH_H__

#define HUGE_VAL         0x7ff0000000000000.0
#define HUGE_VALF        0x7f800000.0f
#define HUGE_VALL        HUGE_VAL

#define NAN              0x7fc00000f

#define INFINITY         HUGE_VALF
#define FP_ILOGB0        0x80000001
#define FP_ILOGBNAN      0x7fffffff

/* POSIX.1-2004 */

#define MAXFLOAT         (3.40282346638528860e+38f)

#define MATH_ERRNO       1
#define MATH_ERREXCEPT   2

#define math_errhandling MATH_ERRNO

#endif /* __BITS_MATH_H__ */

