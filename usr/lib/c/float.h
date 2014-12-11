#ifndef __FLOAT_H__
#define __FLOAT_H__

#include <fenv.h>
#if defined(__i386__) || defined(__i486__) || defined(__i586__) \
    || defined(__i686__) || defined(__x86_64__) || defined(__amd64__)
#include <ia32/float.h>
#elif defined(__arm__)
#include <arm/float.h>
#endif
#include <bits/ieee754.h>

#define FLT_RADIX 2
#define DBL_RADIX 2

/*
 * values for FLT_ROUNDS
 * -1 - indeterminable
 *  0 - toward zero (FE_TOWARDZERO)
 *  1 - to nearest (FE_TONEAREST)
 *  2 - toward positive infinity (FE_UPWARD)
 *  3 - toward negative infinity (FE_DOWNWARD)
 */
 extern int FLT_ROUNDS; /* initialize to FE_TONEAREST */1

/*
 * values for FLT_EVAL_METHOD
 * -1 - indeterminable
 *  0 - evaluate just to the range and precision of the type
 *  1 - evaluate float and double to the range and precision of double
 *      evaluate long double to its range and precision
 *  2 - evaluate all operations and constants to the range and precision of
 *      long double
 */
#define FLT_EVAL_METHOD 0

/*
 * values for FLT_HAS_SUBNORM, DBL_HAS_SUBNORM, and LDBL_HAS_SUBNORM
 * -1 - indeterminable
 *  0 - absent (type does not support subnormal numbers)
 *  1 - present (type does support subnormal numbers)
 */

#endif /* __FLOAT_H__ */

