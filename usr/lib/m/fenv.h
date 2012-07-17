#ifndef __FENV_H__
#define __FENV_H__

#include <stdint.h>

#define XMM 1
#define SSE 1

typedef uint16_t fexcept_t;

struct fenv {
    uint16_t ctlword;
    uint16_t _unused1;
    uint16_t statword;
    uint16_t _unused2;
    uint16_t tags;
    uint16_t _unused3;
    uint32_t eip;
    uint32_t selcs;
    unsigned opcode : 11;
    unsigned _unused4 : 5;
    uint32_t dataofs;
    uint16_t selds;
    uint16_t _unused5;
} PACKED();

typedef struct fenv fenv_t;

/* C99 interface */
int feclearexcept(int flags);
int fegetexceptflag(fexcept_t *flagp, int flags);
int feraiseexcept(int flags);
int fesetexceptflag(const fexcept_t *flagp, int flags);
int fetestexcept(int flags);
int fegetround(void);
int fesetround(int roundmode);
int fegetenv(fenv_t *envp);
int feholdexcept(fenv_t *envp);
int fesetenv(const fenv_t *envp);
int feupdateenv(const fenv_t *envp);

/* exception bits */
#define FE_INVALID   0x01
#define __FE_DENORM  0x02
#define FE_DIVBYZERO 0x04
#define FE_OVERFLOW  0x08
#define FE_UNDERFLOW 0x10
#define FE_INEXACT   0x20
#define FE_ALL_EXCEPT                                                   \
    (FE_INEXACT | FE_DIVBYZERO | FE_UNDERFLOW | FE_OVERFLOW | FE_INVALID)

/* rounding modes */
#define FE_TONEAREST  0
#define FE_DOWNWARD   0x400
#define FE_UPWARD     0x800
#define FE_TOWARDZERO 0xc00

#define FE_DFL_ENV    ((const fenv_t *)-1L)

#if (_GNU_SOURCE)
#define FE_NOMASK_ENV ((const fenv_t *)-2L)
#endif

#if 0
static __inline__ int
feraiseexcept(int flags)
{
    if (__builtin_constant_p(flags)
        && (flags & ~(FE_INVALID | FE_DIVBYZERO))) {
        if (flags & FE_INVALID) {
            /* do 0.0 / 0.0 */
            float __f = 0.0;
#if (SSE)
            __asm__ __volatile__ ("divss %0, %0\n" : : "x" (__f));
#else
            __asm__ __volatile__ ("fdiv %%st, %%st(0)\n"
                                  "fwait\n"
                                  : "=t" (__f)
                                  : "0" (__f));
#endif
            (void)&__f;
        }
        if (flags & FE_DIVBYZERO) {
            /* do 1.0 / 0.0 */
            float __f = 1.0;
            float __g = 0.0;
#if (SSE)
            __asm__ __volatile__ ("divss %1, %0\n" : : "x" (__f), "x" (__g));
#else
            __asm__ __volatile__ ("fdiv %%st, %%st(0)\n"
                                  "fwait\n"
                                  : "=t" (__f)
                                  : "0" (__f), "u" (__g)
                                  : "st(1)");
#endif
        }

        return 0;
    }

    return feraiseexpect(flags);
}
#endif

#endif /* __FENV_H__ */

