
/* @(#)fdlibm.h 1.5 04/04/22 */
/*
 * ====================================================
 * Copyright (C) 2004 by Sun Microsystems, Inc. All rights reserved.
 *
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */

#include "../c/features.h"
#include <stddef.h>
#include <math.h>
#include <float.h>
#include <fenv.h>
#include <errno.h>

/* Sometimes it's necessary to define __LITTLE_ENDIAN explicitly
   but these catch some common cases. */

#if defined(i386) || defined(i486) || \
	defined(intel) || defined(x86) || defined(i86pc) || \
	defined(__alpha) || defined(__osf__)
#define __LITTLE_ENDIAN
#endif

#ifdef __LITTLE_ENDIAN
#define __HI(x) *(1+(int*)&x)
#define __LO(x) *(int*)&x
#define __HIp(x) *(1+(int*)x)
#define __LOp(x) *(int*)x
#else
#define __HI(x) *(int*)&x
#define __LO(x) *(1+(int*)&x)
#define __HIp(x) *(int*)x
#define __LOp(x) *(1+(int*)x)
#endif

#ifdef __STDC__
#define	__PARM(p)	p
#else
#define	__PARM(p)	()
#endif

/*
 * ANSI/POSIX
 */

extern int signgam;

#define	MAXFLOAT	((float)3.40282346638528860e+38)

enum fdversion {fdlibm_ieee = -1, fdlibm_svid, fdlibm_xopen, fdlibm_posix};

#define _LIB_VERSION_TYPE enum fdversion
#define _LIB_VERSION _fdlib_version  

/* if global variable _LIB_VERSION is not desirable, one may 
 * change the following to be a constant by: 
 *	#define _LIB_VERSION_TYPE const enum version
 * In that case, after one initializes the value _LIB_VERSION (see
 * s_lib_version.c) during compile time, it cannot be modified
 * in the middle of a program
 */ 
extern  _LIB_VERSION_TYPE  _LIB_VERSION;

#define _IEEE_  fdlibm_ieee
#define _SVID_  fdlibm_svid
#define _XOPEN_ fdlibm_xopen
#define _POSIX_ fdlibm_posix

#if 0
struct exception {
	int type;
	char *name;
	double arg1;
	double arg2;
	double retval;
};
#endif

#if !defined(HUGE)
#define	HUGE		MAXFLOAT
#endif

/* 
 * set X_TLOSS = pi*2**52, which is possibly defined in <values.h>
 * (one may replace the following line by "#include <values.h>")
 */

#define X_TLOSS		1.41484755040568800000e+16 

#define	DOMAIN		1
#define	SING		2
#define	OVERFLOW	3
#define	UNDERFLOW	4
#define	TLOSS		5
#define	PLOSS		6

/*
 * ANSI/POSIX
 */
extern double acos __PARM((double));
extern double asin __PARM((double));
extern double atan __PARM((double));
extern double atan2 __PARM((double, double));
extern double cos __PARM((double));
extern double sin __PARM((double));
extern double tan __PARM((double));

extern double cosh __PARM((double));
extern double sinh __PARM((double));
extern double tanh __PARM((double));

extern double exp __PARM((double));
extern double frexp __PARM((double, int *));
extern double ldexp __PARM((double, int));
extern double log __PARM((double));
extern double log10 __PARM((double));
extern double modf __PARM((double, double *));

extern double pow __PARM((double, double));
extern double sqrt __PARM((double));

extern double ceil __PARM((double));
extern double fabs __PARM((double));
extern double floor __PARM((double));
extern double fmod __PARM((double, double));

extern double erf __PARM((double));
extern double erfc __PARM((double));
extern double gamma __PARM((double));
extern double hypot __PARM((double, double));
extern int isnan __PARM((double));
extern int finite __PARM((double));
extern double j0 __PARM((double));
extern double j1 __PARM((double));
extern double jn __PARM((int, double));
extern double lgamma __PARM((double));
extern double y0 __PARM((double));
extern double y1 __PARM((double));
extern double yn __PARM((int, double));

extern double acosh __PARM((double));
extern double asinh __PARM((double));
extern double atanh __PARM((double));
extern double cbrt __PARM((double));
extern double logb __PARM((double));
extern double nextafter __PARM((double, double));
extern double remainder __PARM((double, double));
#ifdef _SCALB_INT
extern double scalb __PARM((double, int));
#else
extern double scalb __PARM((double, double));
#endif

#if 0
extern int matherr __PARM((struct exception *));
#endif

/*
 * IEEE Test Vector
 */
extern double significand __PARM((double));

/*
 * Functions callable from C, intended to support IEEE arithmetic.
 */
extern double copysign __PARM((double, double));
extern int ilogb __PARM((double));
extern double rint __PARM((double));
extern double scalbn __PARM((double, int));

/*
 * BSD math library entry points
 */
extern double expm1 __PARM((double));
extern double log1p __PARM((double));

/*
 * Reentrant version of gamma & lgamma; passes signgam back by reference
 * as the second argument; user must allocate space for signgam.
 */
#ifdef _REENTRANT
extern double gamma_r __PARM((double, int *));
extern double lgamma_r __PARM((double, int *));
#endif	/* _REENTRANT */

/* ieee style elementary functions */
extern double __ieee754_sqrt __PARM((double));			
extern double __ieee754_acos __PARM((double));			
extern double __ieee754_acosh __PARM((double));			
extern double __ieee754_log __PARM((double));			
extern double __ieee754_atanh __PARM((double));			
extern double __ieee754_asin __PARM((double));			
extern double __ieee754_atan2 __PARM((double,double));			
extern double __ieee754_exp __PARM((double));
extern double __ieee754_cosh __PARM((double));
extern double __ieee754_fmod __PARM((double,double));
extern double __ieee754_pow __PARM((double,double));
extern double __ieee754_lgamma_r __PARM((double,int *));
extern double __ieee754_gamma_r __PARM((double,int *));
extern double __ieee754_lgamma __PARM((double));
extern double __ieee754_gamma __PARM((double));
extern double __ieee754_log10 __PARM((double));
extern double __ieee754_sinh __PARM((double));
extern double __ieee754_hypot __PARM((double,double));
extern double __ieee754_j0 __PARM((double));
extern double __ieee754_j1 __PARM((double));
extern double __ieee754_y0 __PARM((double));
extern double __ieee754_y1 __PARM((double));
extern double __ieee754_jn __PARM((int,double));
extern double __ieee754_yn __PARM((int,double));
extern double __ieee754_remainder __PARM((double,double));
extern int    __ieee754_rem_pio2 __PARM((double,double*));
#ifdef _SCALB_INT
extern double __ieee754_scalb __PARM((double,int));
#else
extern double __ieee754_scalb __PARM((double,double));
#endif

/* fdlibm kernel function */
extern double __kernel_standard __PARM((double,double,int));	
extern double __kernel_sin __PARM((double,double,int));
extern double __kernel_cos __PARM((double,double));
extern double __kernel_tan __PARM((double,double,int));
extern int    __kernel_rem_pio2 __PARM((double*,double*,int,int,int,const int*));
