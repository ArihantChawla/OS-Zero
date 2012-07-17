#include <stdint.h>
#include <fenv.h>
#include <zero/cdecl.h>

int
feclearexcept(int flags)
{
    fenv_t env;
#if (XMM)
    uint32_t xexc;
#endif

    flags &= FE_ALL_EXCEPT;
    __asm__ __volatile__ ("fnstenv %0\n"
                          "fwait\n"
                          : "=m" (*&env));
    env.statword &= flags ^ FE_ALL_EXCEPT;
    __asm__ __volatile__ ("fldenv %0\n" : : "m" (*&env));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n" : "=m" (*&xexc));
    xexc &= ~flags;
    __asm__ __volatile__ ("ldmxcsr %0\n" : : "m" (*&xexc));
#endif

    return 0;
}

int
fedisableexcept(int flags)
{
    uint32_t exc;
    uint32_t oldexc;
#if (XMM)
    uint32_t xexc;
#endif

    __asm__ ("fstcw %0\n"
             "fwait\n"
             : "=m" (*&exc));
    flags &= FE_ALL_EXCEPT;
    oldexc = (~exc) & FE_ALL_EXCEPT;
    exc |= flags;
    __asm__ __volatile__ ("fldcw %0\n" : : "m" (*&exc));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xexc));
    xexc |= flags << 7;
    __asm__ __volatile__ ("ldmxcsr %0\n" : : "m" (*&xexc));
#endif

    return oldexc;
}

int
feenableexcept(int flags)
{
    uint32_t exc;
    uint32_t oldexc;
#if (XMM)
    uint32_t xexc;
#endif

    __asm__ ("fstcw %0\n"
             "fwait\n"
             : "=m" (*&exc));
    flags &= FE_ALL_EXCEPT;
    oldexc = (~exc) & FE_ALL_EXCEPT;
    exc |= flags;
    __asm__ __volatile__ ("fldcw %0\n" : : "m" (*&exc));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xexc));
    xexc |= ~(flags << 7);
    __asm__ __volatile__ ("ldmxcsr %0\n" : : "m" (*&xexc));
#endif

    return oldexc;
}

int
fegetenv(fenv_t *envp)
{
    __asm__ __volatile__ ("fnstenv %0\n"
                          "fwait\n"
                          : "=m" (*envp));
    /* processor changes the mask; load back */
    __asm__ __volatile__ ("fldenv %0\n" : : "m" (*envp));

    return 0;
}

int
fegetexpect(void)
{
    uint16_t exc;

    __asm__ ("fstcw %0\n"
             "fwait\n"
             : "=m" (*&(exc)));
    exc = (~exc) & FE_ALL_EXCEPT;

    return exc;
}

int
fegetround(void)
{
    int cw;

    __asm__ __volatile__ ("fnstcw %0\n"
                          "fwait\n"
                          : "=m" (*&cw));

    return cw & 0xc00;
}

int
feholdexcept(fenv_t *envp)
{
    fenv_t   env;
#if (XMM)
    uint32_t xval;
#endif

    /* store environment */
    __asm__ __volatile__ ("fnstenv %0\n"
                          "fwait\n"
                          : "=m" (env));
    *envp = env;
    /* set exceptions to non-stop */
    env.ctlword |= 0x3f;
    /* clear all exceptions */
    env.statword &= ~0x3f;
    __asm__ __volatile__ ("fldenv %0\n" : : "m" (env));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xval));
    xval = (xval & 0x1f80) & ~0x3f;
    __asm__ __volatile__ ("ldmxcsr %0\n" : : "m" (*&xval));
#endif

    return 0;
}

int
fesetenv(const fenv_t *envp)
{
    fenv_t env;

    __asm__ __volatile__ ("fnstenv %0\n"
                          "fwait\n"
                          : "=m" (env));
    if (envp == FE_DFL_ENV) {
        env.ctlword |= FE_ALL_EXCEPT;
        env.ctlword &= ~FE_TOWARDZERO;
        env.statword &= ~FE_ALL_EXCEPT;
        env.eip = 0;
        env.selcs = 0;
        env.opcode = 0;
        env.dataofs = 0;
        env.selds = 0;
#if (_GNU_SOURCE)
    } else if (envp == FE_NOMASK_ENV) {
        env.ctlword &= ~(FE_ALL_EXCEPT | FE_TOWARDZERO);
        env.statword &= ~FE_ALL_EXCEPT;
        env.eip = 0;
        env.selcs = 0;
        env.opcode = 0;
        env.dataofs = 0;
        env.selds = 0;
#endif
    } else {
        env.ctlword &= ~(FE_ALL_EXCEPT | FE_TOWARDZERO);
        env.ctlword |= envp->ctlword & (FE_ALL_EXCEPT | FE_TOWARDZERO);
        env.statword &= ~FE_ALL_EXCEPT;
        env.statword |= envp->statword & FE_ALL_EXCEPT;
        env.eip = envp->eip;
        env.selcs = envp->selcs;
        env.opcode = envp->opcode;
        env.dataofs = envp->dataofs;
        env.selds = envp->selds;
    }

    __asm__ __volatile__ ("fldenv %0\n"
                          "fwait\n"
                          :
                          : "m" (env));

    return 0;
}

int
fesetround(int round)
{
    uint16_t cw;
#if (XMM)
    uint32_t xcw;
#endif

    if (round & 0xc00) {
        /* not valid mode */
        return 1;
    }
    __asm__ __volatile__ ("fnstcw %0\n"
                          "fwait\n"
                          : "=m" (*&cw));
    cw &= ~0xc00;
    cw |= round;
    __asm__ __volatile__ ("fldcw %0\n" : : "m" (*&cw));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xcw));
    xcw &= ~0x6000;
    xcw |= round << 3;
    __asm__ ("ldmxcsr %0" : : "m" (*&xcw));
#endif

    return 0;
}

int
feupdateenv(const fenv_t *envp)
{
    uint16_t exc;
    uint32_t xexc;
    
    /* save current exceptions */
    __asm__ __volatile__ ("fnstsw %0\n"
                          "fwait\n"
                          : "=m" (*&exc));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xexc));
#endif
    exc = (exc | xexc) & FE_ALL_EXCEPT;
    /* install new environment */
    fesetenv(envp);
    /* raise saved exception */
    feraiseexcept(exc);

    return 0;
}

int
fegetexceptflag(fexcept_t *flagp, int flags)
{
    fexcept_t exc;
#if (XMM)
    uint32_t  xexc;
#endif

    __asm__ ("fnstsw %0\n"
             "fwait\n"
             : "=m" (*&exc));
    *flagp = exc & flags & FE_ALL_EXCEPT;
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xexc));
    *flagp |= xexc & flags & FE_ALL_EXCEPT;
#endif

    return 0;
}

int
feraiseexcept(int flags)
{
    if (flags & FE_INVALID) {
        /* do 0.0 / 0.0 */
        double x;
        
        __asm__ __volatile__ ("fldz\n"
                              "fdiv %%st, %%st(0)\n"
                              "fwait\n"
                              : "=t" (x));
    }
    if (flags & FE_DIVBYZERO) {
        /* divide by zero */
        double x;
        
        __asm__ __volatile__ ("fldz\n"
                              "fld1\n"
                              "fdivp %%st, %%st(1)\n"
                              "fwait\n"
                              : "=t" (x));
    }
    if (flags & FE_UNDERFLOW) {
        fenv_t env;
        
        __asm__ __volatile__ ("fnstenv %0\n"
                              "fwait\n"
                              : "=m" (*&env));
        env.statword |= FE_UNDERFLOW;
        __asm__ __volatile__ ("fldenv %0\n"
                              "fwait\n"
                              :
                              : "m" (*&env));
    }
    if (flags & FE_INEXACT) {
        fenv_t env;
        
        __asm__ __volatile__ ("fnstenv %0\n"
                              "fwait\n"
                              : "=m" (*&env));
        env.statword |= FE_INEXACT;
        __asm__ __volatile__ ("fldenv %0\n"
                              "fwait\n"
                              :
                              : "m" (*&env));
    }

    return 0;
}

int fesetexceptflg(const fexcept_t *flagp, int flags)
{
    fenv_t   env;
#if (XMM)
    uint32_t xexc;
#endif

    __asm__ __volatile__ ("fnstenv %0\n"
                          "fwait\n"
                          : "=m" (*&env));
    env.statword &= ~(flags & FE_ALL_EXCEPT);
    env.statword |= *flagp & flags & FE_ALL_EXCEPT;
    __asm__ __volatile__ ("fldenv %0\n" : : "m" (*&env));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n" : "=m" (*&xexc));
    xexc &= ~(flags & FE_ALL_EXCEPT);
    xexc |= *flagp & flags & FE_ALL_EXCEPT;
    __asm__ __volatile__ ("ldmxcsr %0\n" : : "m" (*&xexc));
#endif

    return 0;
}

int
fetestexcept(int flags)
{
    uint16_t exc;
    uint32_t xexc;
    int      retval;

    __asm__ __volatile__ ("fnstsw %0\n"
                          "fwait\n"
                          : "=a" (exc));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xexc));
#endif
    retval = (exc | xexc) & flags & FE_ALL_EXCEPT;

    return retval;
}
