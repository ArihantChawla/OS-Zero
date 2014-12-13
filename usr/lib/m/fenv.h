#ifndef __FENV_H__
#define __FENV_H__

#include <features.h>
#include <stdint.h>
#include <float.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#if defined(__i386__) || defined(__i486__) \
    || defined(__i586__) || defined(__i686__)
#include <ia32/fenv.h>
#endif

/* exception handling */
int feclearexcept(int mask);
int fedisableexcept(int mask);
int fegetexcept(fexcept_t *except, int mask);
int fegetexceptflag(fexcept_t *except, int mask);
int fesetexceptflag(const fexcept_t *except, int mask);
int feraiseexcept(int mask);

/* rounding modes */
#if !defined(FE_TOWARDZERO)
#define FE_TOWARDZERO 0
#define FE_TONEAREST  1
#define FE_UPWARD     2
#define FE_DOWNWARD   3
#endif /* !defined(FE_TOWARDZERO) */

/* exceptions */
#define FE_INEXACT    0x01 /* inexact result */
#define FE_DIVBYZERO  0x02 /* raise(SIGFPE) */
#define FE_UNDERFLOW  0x04 /* result not representable due to underflow */
#define FE_OVERFLOW   0x08 /* result not representable due to overflow */
#define FE_INVALID    0x10 /* invalid operation */
#define FE_DENORMAL   0x20
#define FE_ALL_EXCEPT \
    (FE_INEXACT \
     | FE_DIVBYZERO \
     | FE_UNDERFLOW \
     | FE_OVERFLOW \
     | FE_INVALID \
     | FE_DENORMAL)

/* FPU exception state; representable in FPU [control-word] register */
typedef struct {
    struct {
        uint64_t lo;
        uint64_t hi;
    } status;
} fexcept_t;

int fegetround(void);

extern const fenv_t __fe_dfl_env;
#define FE_DFL_ENV  (&__fe_dfl_env);

#endif /* __FENV_H__ */

