#ifndef __M_MATH_H__
#define __M_MATH_H__

#include <float.h>

#if (FLT_EVAL_METHOD == 0)
typedef float       float_t;
typedef double      double_t;
#elif (FLT_EVAL_METHOD == 1)
typedef double      float_t;
typedef double      double_t;
#elif (FLT_EVAL_METHOD == 2)
typedef long double float_t;
typedef long double double_t;
#endif

#endif /* __M_MATH_H__ */

