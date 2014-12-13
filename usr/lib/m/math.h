#ifndef __MATH_H__
#define __MATH_H__

#include <features.h>
#include <float.h>
#include <bits/ieee754.h>
#include <bits/math.h>

#if (FLT_EVAL_METHOD == 0)
typedef float       float_t;
typedef double      double_t;
#define __NO_LONG_DOUBLE_MATH 1
#elif (FLT_EVAL_METHOD == 1)
typedef double      float_t;
typedef double      double_t;
#define __NO_LONG_DOUBLE_MATH 1
#elif (FLT_EVAL_METHOD == 2)
typedef long double float_t;
typedef long double double_t;
#undef  __NO_LONG_DOUBLE_MATH
#endif

/* floating-point number categories */
#define FP_NAN       0
#define FP_INFINITE  1
#define FP_ZERO      2
#define FP_SUBNORMAL 3
#define FP_NORMAL    4
/* the __-prefix macros need to be defined in per-architecture header files */
#define FP_FAST_FMA  __FP_FAST_FMA
#define FP_FAST_FMAF __FP_FAST_FMAF
#define FP_FAST_FMAL __FP_FAST_FMAL

#if defined(USESVID) && (USESVID)

/* type values */
#define DOMAIN    1
#define SING      2
#define OVERFLOW  3
#define UNDERFLOW 4
#define TLOSS     5
#define PLOSS     6

/* return this large value instead of infinity */
#define HUGE      (3.40282347e+38F)

struct exception {
    int     type;
    char   *name;
    double  arg1;
    double  arg2;
    double  retval;
};

extern int matherr(struct exception *exc);

#else /* !USESVID */

#if defined(USEXOPEN) && (USEXOPEN)
#define MAXFLOAT (3.40282347e+38F)
#endif

#endif /* USESVID */

#if (defined(USEBSD) && (USEBSD)) || (defined(USEXOPEN) && (USEXOPEN))

#define M_E         2.7182818284590452354  /* e */
#define M_LOG2E     1.4426950408889634074  /* log_2 e */
#define M_LOG10E    0.43429448190325182765 /* log_10 e */
#define M_LN2       0.69314718055994530942 /* log_e 2 */
#define M_LN10      2.30258509299404568402 /* log_e 10 */
#define M_PI        3.14159265358979323846 /* pi */
#define M_PI_2      1.57079632679489661923 /* pi/2 */
#define M_PI_4      0.78539816339744830962 /* pi/4 */
#define M_1_PI      0.31830988618379067154 /* 1/pi */
#define M_2_PI      0.63661977236758134308 /* 2/pi */
#define M_2_SQRTPI  1.12837916709551257390 /* 2/sqrt(pi) */
#define M_SQRT2	    1.41421356237309504880 /* sqrt(2) */
#define M_SQRT1_2   0.70710678118654752440 /* 1/sqrt(2) */

#endif /* BSD || XOPEN */

#if defined(USEGNU) && (USEGNU)
#define M_El        2.7182818284590452353602874713526625L /* e */
#define M_LOG2El    1.4426950408889634073599246810018922L /* log_2 e */
#define M_LOG10El   0.4342944819032518276511289189166051L /* log_10 e */
#define M_LN2l      0.6931471805599453094172321214581766L /* log_e 2 */
#define M_LN10l     2.3025850929940456840179914546843642L /* log_e 10 */
#define M_PIl       3.1415926535897932384626433832795029L /* pi */
#define M_PI_2l	    1.5707963267948966192313216916397514L /* pi/2 */
#define M_PI_4l	    0.7853981633974483096156608458198757L /* pi/4 */
#define M_1_PIl	    0.3183098861837906715377675267450287L /* 1/pi */
#define M_2_PIl	    0.6366197723675813430755350534900574L /* 2/pi */
#define M_2_SQRTPIl 1.1283791670955125738961589031215452L /* 2/sqrt(pi) */
#define M_SQRT2l    1.4142135623730950488016887242096981L /* sqrt(2) */
#define M_SQRT1_2l  0.7071067811865475244008443621048490L /* 1/sqrt(2) */
#endif /* USEGNU */

#endif /* __MATH_H__ */

