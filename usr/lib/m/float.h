#ifndef __FLOAT_H__
#define __FLOAT_H__

#include <features.h>
#include <bits/ieee754.h>
#if defined(__x86_64__) || defined(__amd64__)                      \
    || defined(__i386__) || defined(__i486__) || defined(__i586__) \
    || defined(__i686__)
#include <ia32/float.h>
#elif defined(__arm__)
#include <arm/float.h>
#elif defined(sparc) || defined(__sparc) || defined(__sparc__)
#include <sparc/float.h>
#endif

#define FLT_RADIX  2
#define DBL_RADIX  FLT_RADIX
#define LDBL_RADIX FLT_RADIX

/*
 * values for FLT_ROUNDS
 * -1 - indeterminable
 *  0 - toward zero (FE_TOWARDZERO)
 *  1 - to nearest (FE_TONEAREST)
 *  2 - toward positive infinity (FE_UPWARD)
 *  3 - toward negative infinity (FE_DOWNWARD)
 */
extern int          fegetround(void);
#define FLT_ROUNDS  fegetround()
/* TODO: fix DBL_ROUNDS and LDBL_ROUNDS */
#define DBL_ROUNDS  FLT_ROUNDS
#define LDBL_ROUNDS FLT_ROUNDS

/*
 * values for FLT_HAS_SUBNORM, DBL_HAS_SUBNORM, and LDBL_HAS_SUBNORM
 * -1 - indeterminable
 *  0 - absent (type does not support subnormal numbers)
 *  1 - present (type does support subnormal numbers)
 */

#endif /* __FLOAT_H__ */

