#define __FPU_FAST_MATH__ 1

#include <stdint.h>
#include <float.h>
#include <fenv.h>
#include <errno.h>
#include <math.h>
#include <ia32/math.h>
#include <zero/trix.h>

int __fpclassify(double x)
{
    union { double d; uint64_t u64; } u = { x };
    int   exp = (uint32_t)((u.u64 & __DBL_MASK) >> 52);

    if (!exp) {
        if (u.u64 & __DBL_MANTBITS) {

            return FP_SUBNORMAL;
        }

        return FP_ZERO;
    }
    if (exp == 0x7ff) {
        if (u.u64 & __DBL_MANTBITS) {

            return FP_NAN;
        }

        return FP_INFINITE;
    }

    return FP_NORMAL;
}

int __fpclassifyf(float x)
{
    union { float d; uint64_t u32; } u = { x };
    int   exp = (u.u32 & __FLT_MASK) >> 23;

    if (!exp) {
        if (u.u32 & __FLT_MANTBITS) {

            return FP_SUBNORMAL;
        }

        return FP_ZERO;
    }
    if (exp == 0xff) {
        if (u.u32 & __FLT_MANTBITS) {

            return FP_NAN;
        }

        return FP_INFINITE;
    }

    return FP_NORMAL;
}

int __fpclassifyl(long double x)
{
    uint64_t mant = ldgetmant(x);
    int      exp = ldgetexp(x);

    if (!exp) {
        if (mant) {

            return FP_SUBNORMAL;
        }

        return FP_ZERO;
    }
    if (exp == 0xff) {
        if (mant) {

            return FP_NAN;
        }

        return FP_INFINITE;
    }

    return FP_NORMAL;
}

/* TODO: lots of work to be done here...
 * - initialise FPU environment; set rounding mode etc.
 */

double
#if (MATHTEST)
zsqrt(double x)
#else
sqrt(double x)
#endif
{
    double retval = 0.0;

    if (x < 0.0) {
        dsetnan(retval);
    } else if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (!dgetsign(x) && fpclassify(x) == FP_INFINITE) {
        retval = dsetexp(retval, 0x7ff);
    } else if (x < -0.0) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            dsetsnan(retval);
        } else {
            dsetnan(retval);
        }
    } else {
        __fpusqrt(x, retval);
    }
        
    return retval;
}

double
#if (MATHTEST)
zsin(double x)
#else
sin(double x)
#endif
{
    double retval;

    if (isnan(x)) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            dsetsnan(retval);
        } else {
            dsetnan(retval);
        }
    } else {
        __fpusin(x, retval);
    }
        
    return retval;
}

double
#if (MATHTEST)
zcos(double x)
#else
cos(double x)
#endif
{
    double retval;

    /* TODO: error handling */
    __fpucos(x, retval);

    return retval;
}

double
#if (MATHTEST)
ztan(double x)
#else
tan(double x)
#endif
{
    double tmp;
    double retval;

    if (isnan(x)) {
        retval = x;
    } else if (fpclassify(x) == FP_ZERO) {
        fsetnan(retval);
    } else if (fpclassify(x) == FP_INFINITE) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            dsetsnan(retval);
        } else {
            dsetnan(retval);
        }
    } else {
        __fputan(x, retval);
        if (dgetsign(retval) && isnan(retval)) {
            retval = 0.0;
        }
    }

    return retval;
}

/* FIXME: atan() doesn't work yet */
double
#if (MATHTEST)
zatan(double x)
#else
atan(double x)
#endif
{
    double retval;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        if (dgetsign(x)) {
            retval = -M_PI_2;
        } else {
            retval = M_PI_2;
        }
    } else {
        __fpuatan(x, retval);
    }

    return retval;
}

#if ((_BSD_SOURCE) || (_SVID_SOURCE) || _XOPEN_SOURCE >= 600            \
    || (_ISOC99_SOURCE) || _POSIX_C_SOURCE >= 200112L)

float
/* FIXME: sqrtf doesn't work yet */
#if (MATHTEST)
zsqrtf(float x)
#else
sqrtf(float x)
#endif
{
    float retval = 0.0;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (!dgetsign(x) && fpclassify(x) == FP_INFINITE) {
        retval = fsetexp(retval, 0xff);
    } else if (x < -0.0) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            retval = fsetsnan(x);
        } else {
            retval = fsetnan(x);
        }
    } else {
        __asm__ __volatile__ ("flds %0\n" : : "m" (x));
        __asm__ __volatile__ ("fsqrt\n");
        __asm__ __volatile__ ("fstps %0\n"
                              "fwait\n"
                              : "=m" (retval));
    }
        
    return retval;
}

float
#if (MATHTEST)
zsinf(float x)
#else
sinf(float x)
#endif
{
    float retval;

    __fpusinf(x, retval);

    return retval;
}

float
#if (MATHTEST)
zcosf(float x)
#else
cosf(float x)
#endif
{
    float retval;

    __fpucosf(x, retval);

    return retval;
}

float
#if (MATHTEST)
ztanf(float x)
#else
tanf(float x)
#endif
{
    uint32_t fcw;
    uint32_t tmp;
    float    retval;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        if (dgetsign(x)) {
            retval = -M_PI_2;
        } else {
            retval = M_PI_2;
        }
    } else {
        __asm__ __volatile__ ("fstcw %0\n"
                              "movw %0, %%dx\n"
                              "orw $0x0800, %%dx\n"
                              "andw $0xfbff, %%dx\n"
                              "movw %%dx, %1\n"
                              "fldcw %1\n"
                              "flds %3\n"
                              "fptan\n"
                              "fstps %2\n"
                              "fstps %2\n"
                              "fldcw %0\n"
                              "fwait\n"
                              : "=m" (fcw), "=m" (tmp), "=m" (retval)
                              : "m" (x)
                              : "edx");
        if (fgetsign(retval) && isnan(retval)) {
            retval = 0.0;
        }
    }

    return retval;
}

/* TODO: sqrtl() doesn't work yet */
long double
#if (MATHTEST)
zsqrtl(long double x)
#else
sqrtl(long double x)
#endif
{
    long double retval = 0.0;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (!dgetsign(x) && fpclassify(x) == FP_INFINITE) {
        retval = fsetexp(retval, 0xff);
    } else if (x < -0.0) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            ldsetnan(retval);
        } else {
            ldsetnan(retval);
        }
    } else {
        __asm__ __volatile__ ("fldt %0\n" : : "m" (x));
        __asm__ __volatile__ ("fsqrt\n");
        __asm__ __volatile__ ("fstpt %0\n"
                              "fwait\n"
                              : "=m" (retval));
    }
        
    return retval;
}

long double
#if (MATHTEST)
zsinl(long double x)
#else
sinl(long double x)
#endif
{
    long double retval;

    __fpusinl(x, retval);

    return retval;
}

long double
#if (MATHTEST)
zcosl(long double x)
#else
cosl(long double x)
#endif
{
    long double retval;

    __fpucosl(x, retval);

    return retval;
}

/* TODO: tanl() doesn't work yet */
long double
#if (MATHTEST)
ztanl(long double x)
#else
tanl(long double x)
#endif
{
    long double tmp;
    long double retval;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        retval = 0.0;
        errno = EDOM;
        feraiseexcept(FE_INVALID);
    } else {
        __asm__ __volatile__ ("fldt %0\n" : : "m" (x));
        __asm__ __volatile__ ("fptan\n");
        __asm__ __volatile__ ("fstpt %0\n" : "=m" (tmp));
        __asm__ __volatile__ ("fstpt %0\n"
                              "fwait\n"
                              : "=m" (retval));
        if (ldgetsign(retval) && isnan(retval)) {
            retval = 0.0;
        }
    }

    return retval;
}

#endif

#if (_GNU_SOURCE)
#if (MATHTEST)
void
zsincos(double x, double *sin, double *cos)
#else
void
sincos(double x, double *sin, double *cos)
#endif
{
    __fpusin(x, sin);
    __fpucos(x, cos);
//    __fpusincos(x, sin, cos);

    return;
}

#if (MATHTEST)
void
zsincosf(float x, float *sin, float *cos)
#else
void
sincosf(float x, float *sin, float *cos)
#endif
{
    __fpusincosf(x, sin, cos);

    return;
}

#if (MATHTEST)
void
zsincosl(long double x, long double *sin, long double *cos)
#else
void
sincosl(long double x, long double *sin, long double *cos)
#endif
{
    __fpusincosl(x, sin, cos);

    return;
}
#endif

#if (MATHTEST)

#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#define RADMAX (1024 * 1024)
int
main(int argc,
     char *argv[])
{
    double      d;
    double      d1;
    double      d2;
    float       f;
    float       f1;
    float       f2;
    long double ld;
    long double ld1;
    long double ld2;
    double      sin1;
    double      sin2;
    double      cos1;
    double      cos2;
    float       sinf1;
    float       sinf2;
    float       cosf1;
    float       cosf2;
    long double sinld1;
    long double sinld2;
    long double cosld1;
    long double cosld2;
    fenv_t      fenv;

    fegetenv(&fenv);
    fesetenv(FE_DFL_ENV);
    fegetenv(&fenv);
    fprintf(stderr, "sin 0 == %e\n", sin(0));
    for ( d = -RADMAX ; d < RADMAX ; d += 0.125 ) {
        d1 = sqrt(d);
        d2 = zsqrt(d);
        if (d1 != d2 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "SQRT failure (%e: %ex != %e)\n", d, d1, d2);

            exit(1);
        }
#if 0
        sincos(d, &sin1, &cos1);
        zsincos(d, &sin2, &cos2);
        if (sin1 != sin2 || cos1 != cos2) {
            fprintf(stderr, "SINCOS failure (%e/%e/%e) -> (%e/%e/%e)\n",
                d, sin1, cos1, d, sin2, cos2);

            exit(1);
        }
#endif
        d1 = sin(d);
        d2 = zsin(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "SIN(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
        d1 = cos(d);
        d2 = zcos(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "COS(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
        d1 = tan(d);
        d2 = ztan(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "TAN(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
        d1 = atan(d);
        d2 = zatan(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "ATAN(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
    }

    for ( ld = -100.0 ; ld < 100.0 ; ld += 1.0 ) {
        ld1 = sqrtl(ld);
        ld2 = zsqrtl(ld);
        if (ld1 != ld2) {
            fprintf(stderr, "SQRTL(%Lf) failure(%Lf/%Lf)\n", ld, ld2, ld1);

            exit(1);
        }
#if 0
        sincosl(ld, &sinld1, &cosld1);
        zsincosl(ld, &sinld2, &cosld2);
        if ((sinld1 != sinld2 || cosld1 != cosld2)
            && (fpclassify(sinld1) != fpclassify(sinld2))) {
            fprintf(stderr, "SINCOSL failure (%Lf/%Lf/%Lf) -> (%Lf/%Lf/%Lf\n",
                    ld, sinld1, cosld1, ld, sinld2, cosld2);

            exit(1);
        }
#endif
        ld1 = sinl(ld);
        ld2 = zsinl(ld);
        if (ld2 != ld1 && (fpclassify(ld1) != fpclassify(ld2))) {
            fprintf(stderr, "SINLD: ERROR(%Le) %Le should be %Le\n", ld, ld1, ld2);
                
            return 1;
#if 0
        } else {
                fprintf(stderr, "SIN(%Le) == %Le\n", ld, ld2);
#endif
        }
        ld1 = cosl(ld);
        ld2 = zcosl(ld);
        if (ld2 != ld1 && (fpclassify(ld1) != fpclassify(ld2))) {
            fprintf(stderr, "COSLD: ERROR(%Le) %Le should be %Le\n", ld, ld2, ld1);
            
            return 1;
#if 0
        } else {
            fprintf(stderr, "COSLD(%Le) == %Le\n", ld, ld2);
#endif
        }
        ld1 = tanl(ld);
        ld2 = ztanl(ld);
        if (ld2 != ld1 && (fpclassify(ld1) != fpclassify(ld2))) {
            fprintf(stderr, "TANL: ERROR(%Le) %Le should be %Le\n", ld, ld2, ld1);
            
            return 1;
#if 0
        } else {
            fprintf(stderr, "TANF(%Le) == %Le\n", ld, ld2);
#endif
        }
    }

    for ( f = -100.0 ; f < 100.0 ; f += 1.0 ) {
        f1 = sqrtf(f);
        f2 = zsqrtf(f);
        if (f1 != f2) {
            fprintf(stderr, "SQRTF(%e) failure(%e/%e)\n", f, f2, f1);

            exit(1);
        }
#if 0
        sincosf(f, &sinf1, &cosf1);
        zsincosf(f, &sinf2, &cosf2);
        if (sinf1 != sinf2 || cosf1 != cosf2) {
            fprintf(stderr, "SINCOSF failure (%e/%e/%e) -> (%e/%e/%e)\n",
                f, sinf1, cosf1, f, sinf2, cosf2);

            exit(1);
        }
#endif
        f1 = sinf(f);
        f2 = zsinf(f);
        if (f2 != f1) {
            fprintf(stderr, "SINF: ERROR(%e) %e should be %e\n", f, f1, f2);
                
            return 1;
#if 0
        } else {
                fprintf(stderr, "SIN(%e) == %e\n", f, f2);
#endif
        }
        f1 = cosf(f);
        f2 = zcosf(f);
        if (f2 != f1) {
            fprintf(stderr, "COSF: ERROR(%e) %e should be %e\n", f, f2, f1);
            
            return 1;
#if 0
        } else {
            fprintf(stderr, "COS(%e) == %e\n", f, f2);
#endif
        }
        f1 = tanf(f);
        f2 = ztanf(f);
        if (f2 != f1) {
            fprintf(stderr, "TANF: ERROR(%e) %e should be %e\n", f, f2, f1);
            
            return 1;
#if 0
        } else {
            fprintf(stderr, "TANF(%e) == %e\n", f, f2);
#endif
        }
    }

    return 0;
}

#endif /* MATHTEST */

