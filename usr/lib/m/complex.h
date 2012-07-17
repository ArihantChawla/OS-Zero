#ifndef __COMPLEX_H__
#define __COMPLEX_H__

#include <features.h>

#if (_ISO_C99_SOURCE)

double      cabs(double complex z);
float       cabsf(double complex z);
long double cabsl(double complex z);
double      carg(double complex z);
float       cargf(double complex z);
long double cargl(double complex z);

#endif /* _ISO_C99_SOURCE */

#endif /* __COMPLEX_H__ */

