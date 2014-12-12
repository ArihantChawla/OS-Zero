#ifndef __FLOAT_H__
#define __FLOAT_H__

#include <bits/ieee754.h>
#if defined(__x86_64__) || defined(__amd64__)
#include <x86-64/float.h>
#elif defined(__i386__) || defined(__i486__) || defined(__i586__) \
    || defined(__i686__)
#include <ia32/float.h>
#elif defined(__arm__)
#include <arm/float.h>
#elif defined(sparc) || defined(__sparc) || defined(__sparc__)
#include <sparc/float.h>
#endif
#include <fenv.h>

#define FLT_RADIX  2
#define DBL_RADIX  2
#define LDBL_RADIX 2

/*
 * values for FLT_ROUNDS
 * -1 - indeterminable
 *  0 - toward zero (FE_TOWARDZERO)
 *  1 - to nearest (FE_TONEAREST)
 *  2 - toward positive infinity (FE_UPWARD)
 *  3 - toward negative infinity (FE_DOWNWARD)
 */
#if defined(sparc) || defined(__sparc) || defined(__sparc__)
#if defined(__STDC__)
extern int __flt_rounds(void);
#else /* !defined(__STDC__) */
extern int __flt_rounds();
#endif /* defined(__STDC__) */
#define FLT_ROUNDS __flt_rounds()
#else /* !sparc */
extern int __flt_rounds;
#define FLT_ROUNDS __flt_rounds
#if defined(__STDC__)
extern int __fltrounds(void);
#else /* !defined(__STDC__) */
extern int __fltrounds();
#endif /* defined(__STDC__) */
#endif /* sparc */
/* TODO: fix DBL_ROUNDS and LDBL_ROUNDS */
extern int DBL_ROUNDS;
extern int LDBL_ROUNDS;

/*
 * values for FLT_HAS_SUBNORM, DBL_HAS_SUBNORM, and LDBL_HAS_SUBNORM
 * -1 - indeterminable
 *  0 - absent (type does not support subnormal numbers)
 *  1 - present (type does support subnormal numbers)
 */

#endif /* __FLOAT_H__ */

