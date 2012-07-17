#ifndef __MATH_H__
#define __MATH_H__

#include <stdint.h>
#include <zero/trix.h>

double      sqrt(double x);
double      sin(double x);
double      cos(double x);
double      tan(double x);
double      asin(double x);
double      acos(double x);
double      atan(double x);
#if ((_BSD_SOURCE) || (_SVID_SOURCE) || _XOPEN_SOURCE >= 600            \
    || (_ISOC99_SOURCE) || _POSIX_C_SOURCE >= 200112L)
float       sinf(float x);
long double sinl(long double x);
float       cosf(float x);
long double cosl(long double x);
float       tanf(float x);
long double tanl(long double x);
float       asinf(float x);
long double asinl(long double x);
float       acosf(float x);
long double acosl(long double x);
float       atanf(float x);
long double atanl(long double x);
#endif
#if (_GNU_SOURCE)
void        sincos(double x, double *sin, double *cos);
void        sincosf(float x, float *sin, float *cos);
void        sincosl(long double x, long double *sin, long double *cos);
#endif

#define M_PI 3.14159265358979323846
#define HUGE_VALL (__extension__ 0x10p32767L)
#define HUGE_VAL  ((double)(UINT64_C0x7ff0000000000000))
#define HUGE_VALF ((float)(UINT32_T(0x7f800000)))

#define isnormal(x) (fpclassify(x) == FP_NORMAL)
#define isfinite(x) (fpclassify(x) != FP_NAN && fpclassify(x) != FP_INFINITE)
#define isnan(x)    (fpclassify(x) == FP_NAN)
#define isinf(x)    ((fpclassify(x) == FP_INFINITE)                     \
                     ? ((_getsign(x))                                   \
                        ? -1                                            \
                        : 1)                                            \
                     0)
#define _getsign(x)                                                     \
    ((sizeof(x) == sizeof(float)                                        \
      ? fgetsign(x)                                                     \
      : ((sizeof(x) == sizeof(double))                                  \
         ? dgetsign(x)                                                  \
         : ldgetsign(x))))

/* value classification */
#define FP_NORMAL    0
#define FP_ZERO      1
#define FP_INFINITE  2
#define FP_NAN       3
#define FP_SUBNORMAL 4

static __inline__ int
_fpclassify(double x)
{
    uint32_t sign = dgetsign(x);
    uint64_t mant = dgetmant(x);
    uint32_t exp = dgetexp(x);
    int      retval = FP_NORMAL;

    if (!mant && !exp) {
        retval = FP_ZERO;
    } else if (exp == 0x7ff) {
        if (!mant) {
            if (sign) {
                retval = -FP_INFINITE;
            } else {
                retval = FP_INFINITE;
            }
        } else  {
            retval = FP_NAN;
        }
    } else if (!exp) {
        retval = FP_SUBNORMAL;
    }

    return retval;
}

static __inline__ int
_fpclassifyf(float x)
{
    uint32_t sign = fgetsign(x);
    uint32_t mant = fgetmant(x);
    uint32_t exp = fgetexp(x);
    int      retval = FP_NORMAL;

    if (!mant && !exp) {
        retval = FP_ZERO;
    } else if (exp == 0xff) {
        if (!mant) {
            if (sign) {
                retval = -FP_INFINITE;
            } else {
                retval = FP_INFINITE;
            }
        } else  {
            retval = FP_NAN;
        }
    } else if (!exp) {
        retval = FP_SUBNORMAL;
    }

    return retval;
}

static __inline__ int
_fpclassifyl(float x)
{
    uint32_t sign = ldgetsign(x);
    uint64_t mant = ldgetmant(x);
    uint32_t exp = ldgetexp(x);
    int      retval = FP_NORMAL;

    if (!mant && !exp) {
        retval = FP_ZERO;
    } else if (exp == 0x7fff) {
        if (!mant) {
            if (sign) {
                retval = -FP_INFINITE;
            } else {
                retval = FP_INFINITE;
            }
        } else  {
            retval = FP_NAN;
        }
    } else if (!exp) {
        retval = FP_SUBNORMAL;
    }

    return retval;
}

#define fpclassify(x)                                                   \
    ((sizeof(x) == sizeof(float))                                       \
     ? _fpclassifyf(x)                                                  \
     : ((sizeof(x) == sizeof(double))                                   \
        ? _fpclassify(x)                                                \
        : _fpclassifyl(x)))

#endif /* __MATH_H__ */

