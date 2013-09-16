#if (MATHTEST)
#include <stdio.h>
#include <stdlib.h>
#endif
#include <features.h>
#include <fenv.h>
#include <errno.h>
#include <math.h>
#include <i387/math.h>
#include <zero/trix.h>

/* TODO: lots of work to be done here...
 * - initialise FPU environment; set rounding mode etc.
 */

__inline__ double
#if (MATHTEST)
_sqrt(double x)
#else
sqrt(double x)
#endif
{
    double retval;

    if (x < 0) {
        retval = getnan(x);
    } else if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (!dgetsign(x) && fpclassify(x) == FP_INFINITE) {
        retval = dsetexp(retval, 0x7ff);
    } else if (x < -0.0) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            retval = getsnan(x);
        } else {
            retval = getnan(x);
        }
    } else {
        __asm__ __volatile__ ("fldl %0\n" : : "m" (x));
        __asm__ __volatile__ ("fsqrt\n");
        __asm__ __volatile__ ("fstpl %0\n"
                              "fwait\n"
                              : "=m" (retval));
    }
        
    return retval;
}

__inline__ double
#if (MATHTEST)
_sin(double x)
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
            retval = getsnan(x);
        } else {
            retval = getnan(x);
        }
    } else {
        __asm__ __volatile__ ("fldl %0\n" : : "m" (x));
        __asm__ __volatile__ ("fsin\n");
        __asm__ __volatile__ ("fstpl %0\n"
                              "fwait\n"
                              : "=m" (retval));
    }
        
    return retval;
}

__inline__ double
#if (MATHTEST)
_cos(double x)
#else
cos(double x)
#endif
{
    double retval;

    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));
    __asm__ __volatile__ ("fcos\n");
    __asm__ __volatile__ ("fstpl %0\n"
                          "fwait\n"
                          : "=m" (retval));

    return retval;
}

__inline__ double
#if (MATHTEST)
_tan(double x)
#else
tan(double x)
#endif
{
    double tmp;
    double retval;

    if (isnan(x)) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            retval = getsnan(x);
        } else {
            retval = getnan(x);
        }
    } else {
        __asm__ __volatile__ ("fldl %0\n" : : "m" (x));
        __asm__ __volatile__ ("fptan\n");
        __asm__ __volatile__ ("fstpl %0\n" : "=m" (tmp));
        __asm__ __volatile__ ("fstpl %0\n"
                              "fwait\n"
                              : "=m" (retval));
        if (dgetsign(retval) && isnan(retval)) {
            retval = 0.0;
        }
    }

    return retval;
}

/* FIXME: atan() doesn't work yet */
#if 0
__inline__ double
#if (MATHTEST)
_atan(double x)
#else
atan(double x)
#endif
{
    double retval;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        if (dgetsign(x)) {
            retval = -M_PI * 0.5;
        } else {
            retval = M_PI * 0.5;
        }
    } else {
        __asm__ __volatile__ ("fldl %0\n" : : "m" (x));
        __asm__ __volatile__ ("fpatan\n");
        __asm__ __volatile__ ("fstpl %0\n"
                              "fwait\n"
                              : "=m" (retval));
    }

    return retval;
}
#endif

#if ((_BSD_SOURCE) || (_SVID_SOURCE) || _XOPEN_SOURCE >= 600            \
    || (_ISOC99_SOURCE) || _POSIX_C_SOURCE >= 200112L)

/* FIXME: sqrtf doesn't work yet */
#if 0
#if (MATHTEST)
_sqrtf(float x)
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
            retval = getsnanf(x);
        } else {
            retval = getnanf(x);
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
#endif

__inline__ float
#if (MATHTEST)
_sinf(float x)
#else
sinf(float x)
#endif
{
    float retval;

    __asm__ __volatile__ ("flds %0\n" : : "m" (x));
    __asm__ __volatile__ ("fsin\n");
    __asm__ __volatile__ ("fstps %0\n"
                          "fwait\n"
                          : "=m" (retval));

    return retval;
}

__inline__ float
#if (MATHTEST)
_cosf(float x)
#else
cosf(float x)
#endif
{
    float retval;

    __asm__ __volatile__ ("flds %0\n" : : "m" (x));
    __asm__ __volatile__ ("fcos\n");
    __asm__ __volatile__ ("fstps %0\n"
                          "fwait\n"
                          : "=m" (retval));
    return retval;
}

__inline__ float
#if (MATHTEST)
_tanf(float x)
#else
tanf(float x)
#endif
{
    float tmp;
    float retval;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        if (dgetsign(x)) {
            retval = -M_PI * 0.5;
        } else {
            retval = M_PI * 0.5;
        }
    } else {
        __asm__ __volatile__ ("flds %0\n" : : "m" (x));
        __asm__ __volatile__ ("fptan\n");
        __asm__ __volatile__ ("fstps %0\n" : "=m" (tmp));
        __asm__ __volatile__ ("fstps %0\n"
                              "fwait\n"
                              : "=m" (retval));
        if (fgetsign(retval) && isnan(retval)) {
            retval = 0.0;
        }
    }

    return retval;
}

/* TODO: sqrtl() doesn't work yet */
#if 0
#if (MATHTEST)
_sqrtl(long double x)
#else
sqrtl(long double x)
#endif
{
    long double retval = 0.0;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (!dgetsign(x) && fpclassify(x) == FP_INFINITE) {
        retval = fsetexp(retval, 0x7fff);
    } else if (x < -0.0) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            retval = getsnanl(x);
        } else {
            retval = getnanl(x);
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
#endif

__inline__ long double
#if (MATHTEST)
_sinl(long double x)
#else
sinl(long double x)
#endif
{
    long double retval;

    __asm__ __volatile__ ("fldt %0\n" : : "m" (x));
    __asm__ __volatile__ ("fsin\n");
    __asm__ __volatile__ ("fstpt %0\n"
                          "fwait\n"
                          : "=m" (retval));

    return retval;
}

__inline__ long double
#if (MATHTEST)
_cosl(long double x)
#else
cosl(long double x)
#endif
{
    long double retval;

    __asm__ __volatile__ ("fldt %0\n" : : "m" (x));
    __asm__ __volatile__ ("fcos\n");
    __asm__ __volatile__ ("fstpt %0\n"
                          "fwait\n"
                          : "=m" (retval));

    return retval;
}

/* TODO: tanl() doesn't work yet */
#if 0
__inline__ long double
#if (MATHTEST)
_tanl(long double x)
#else
tanl(long double x)
#endif
{
    long double tmp;
    long double retval;

    if (isnan(x) || fpclassify(x) == FP_ZERO) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
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

#endif

#if (_GNU_SOURCE)
#if (MATHTEST)
void
_sincos(double x, double *sin, double *cos)
#else
void
sincos(double x, double *sin, double *cos)
#endif
{
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));
    __asm__ __volatile__ ("fsincos\n");
    __asm__ __volatile__ ("fstpl %0\n"
                          "fwait\n"
                          : "=m" (*cos));
    __asm__ __volatile__ ("fstpl %0\n"
                          "fwait\n"
                          : "=m" (*sin));

    return;
}

#if (MATHTEST)
void
_sincosf(float x, float *sin, float *cos)
#else
void
sincosf(float x, float *sin, float *cos)
#endif
{
    __asm__ __volatile__ ("flds %0\n" : : "m" (x));
    __asm__ __volatile__ ("fsincos\n");
    __asm__ __volatile__ ("fstps %0\n"
                          "fwait\n"
                          : "=m" (*cos));
    __asm__ __volatile__ ("fstps %0\n"
                          "fwait\n"
                          : "=m" (*sin));

    return;
}

#if (MATHTEST)
void
_sincosl(long double x, long double *sin, long double *cos)
#else
void
sincosl(long double x, long double *sin, long double *cos)
#endif
{
    __asm__ __volatile__ ("fldt %0\n" : : "m" (x));
    __asm__ __volatile__ ("fsincos\n");
    __asm__ __volatile__ ("fstpt %0\n"
                          "fwait\n"
                          : "=m" (*cos));
    __asm__ __volatile__ ("fstpt %0\n"
                          "fwait\n"
                          : "=m" (*sin));

    return;
}
#endif

#if (MATHTEST)
#define RADMAX (512 * 1024)
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

    fprintf(stderr, "sin 0 == %f\n", sin(0));
    for ( d = -RADMAX ; d < RADMAX ; d += 0.125 ) {
        d1 = sqrt(d);
        d2 = _sqrt(d);
        if (d1 != d2 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "SQRT failure (%e: %ex != %e)\n", d, d1, d2);

            exit(1);
        }
#if 0
        sincos(d, &sin1, &cos1);
        _sincos(d, &sin2, &cos2);
        if (sin1 != sin2 || cos1 != cos2) {
            fprintf(stderr, "SINCOS failure (%f/%f/%f) -> (%f/%f/%f)\n",
                d, sin1, cos1, d, sin2, cos2);

            exit(1);
        }
#endif
        d1 = sin(d);
        d2 = _sin(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "SIN(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
        d1 = cos(d);
        d2 = _cos(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "COS(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
        d1 = tan(d);
        d2 = _tan(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "TAN(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
#if 0
        d1 = atan(d);
        d2 = _atan(d);
        if (d2 != d1 && (fpclassify(d1) != fpclassify(d2))) {
            fprintf(stderr, "ATAN(%e): ERROR %e should be %e\n", d, d2, d1);
            
            return 1;
        }
#endif
    }

    for ( ld = -100.0 ; ld < 100.0 ; ld += 1.0 ) {
#if 0
        ld1 = sqrtl(ld);
        ld2 = _sqrtl(ld);
        if (ld1 != ld2) {
            fprintf(stderr, "SQRTL(%Lf) failure(L%f/L%f)\n", ld, ld2, ld1);

            exit(1);
        }
#endif
        sincosl(ld, &sinld1, &cosld1);
        _sincosl(ld, &sinld2, &cosld2);
        if ((sinld1 != sinld2 || cosld1 != cosld2)
            && (fpclassify(sinld1) != fpclassify(sinld2))) {
            fprintf(stderr, "SINCOSL failure (%Lf/%Lf/%Lf) -> (%Lf/%Lf/%Lf\n",
                    ld, sinld1, cosld1, ld, sinld2, cosld2);

            exit(1);
        }
        ld1 = sinl(ld);
        ld2 = _sinl(ld);
        if (ld2 != ld1 && (fpclassify(ld1) != fpclassify(ld2))) {
            fprintf(stderr, "SINLD: ERROR(%Le) %Le should be %Le\n", ld, ld1, ld2);
                
            return 1;
#if 0
        } else {
                fprintf(stderr, "SIN(%Le) == %Le\n", ld, ld2);
#endif
        }
        ld1 = cosl(ld);
        ld2 = _cosl(ld);
        if (ld2 != ld1 && (fpclassify(ld1) != fpclassify(ld2))) {
            fprintf(stderr, "COSLD: ERROR(%Le) %Le should be %Le\n", ld, ld2, ld1);
            
            return 1;
#if 0
        } else {
            fprintf(stderr, "COSLD(%Le) == %Le\n", ld, ld2);
#endif
        }
#if 0
        ld1 = tanl(ld);
        ld2 = _tanl(ld);
        if (ld2 != ld1 && (fpclassify(ld1) != fpclassify(ld2))) {
            fprintf(stderr, "TANL: ERROR(%Le) %Le should be %Le\n", ld, ld2, ld1);
            
            return 1;
#if 0
        } else {
            fprintf(stderr, "TANF(%Le) == %Le\n", ld, ld2);
#endif
        }
#endif
    }

    for ( f = -100.0 ; f < 100.0 ; f += 1.0 ) {
#if 0
        f1 = sqrtf(f);
        f2 = _sqrtf(f);
        if (f1 != f2) {
            fprintf(stderr, "SQRTF(%f) failure(%f/%f)\n", f, f2, f1);

            exit(1);
        }
#endif
        sincosf(f, &sinf1, &cosf1);
        _sincosf(f, &sinf2, &cosf2);
        if (sinf1 != sinf2 || cosf1 != cosf2) {
            fprintf(stderr, "SINCOSF failure (%f/%f/%f) -> (%f/%f/%f)\n",
                f, sinf1, cosf1, f, sinf2, cosf2);

            exit(1);
        }
        f1 = sinf(f);
        f2 = _sinf(f);
        if (f2 != f1) {
            fprintf(stderr, "SINF: ERROR(%e) %e should be %e\n", f, f1, f2);
                
            return 1;
#if 0
        } else {
                fprintf(stderr, "SIN(%e) == %e\n", f, f2);
#endif
        }
        f1 = cosf(f);
        f2 = _cosf(f);
        if (f2 != f1) {
            fprintf(stderr, "COSF: ERROR(%e) %e should be %e\n", f, f2, f1);
            
            return 1;
#if 0
        } else {
            fprintf(stderr, "COS(%e) == %e\n", f, f2);
#endif
        }
        f1 = tanf(f);
        f2 = _tanf(f);
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

