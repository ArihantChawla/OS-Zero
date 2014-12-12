#include <stdint.h>
#include <fenv.h>
#include <zero/cdecl.h>

int
fedisableexcept(int mask)
{
    int mxcsr;
    int oldmask;
    uint16_t control;
    
    mask &= FE_ALL_EXCEPT;
    __i387fnstcw(&control);
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
    } else {
        mxcsr = 0;
    }
    omask = ~(control | (mxcsr >> __SSE_EXCEPT_SHIFT)) & FE_ALL_EXCEPT;
    control |= mask;
    __i387fldcw(control);
    if (__HAS_SSE()) {
        mxcsr |= mask << __SSE_EXCEPT_SHIFT;
        __sseldmxcsr(mxcsr);
    }

    return oldmask;
}

int
feenableexcept(int mask)
{
    int      oldmask;
    int      __mxcsr;
    uint16_t __status;
    uint16_t __control;

    mask &= FE_ALL_EXCEPT;
    __i387fnstsw(&__control;);
    if (__sse_online()) {
        __ssestmxcsr(&__mxcsr);
    } else {
        __mxcsr = 0;
    }
    __mxcsr >>= __SSE_EXCEPT_SHIFT;
    __oldmask = ~(__control | __mxcsr) & FE_ALL_EXCEPT;
    __control &= ~mask;
    __i387fldcw(__control);
    if (SSE_SUPPORTED()) {
        __mxcsr &= ~(mask << __SSE_EXCEPT_SHIFT);
        __sseldmxcsr(__mxcsr);
    }

    return __oldmask;
}

int
fegetenv(fenv_t *env)
{
    int mxcsr;

    /* masks all exceptions */
    __i387fnstenv(env);
    /* restore control word */
    __i387fldcw(env->__control);
    if (__sse_online()) {
        __ssestmxcsr(mxcsr);
        __fesetmxcsr(*env, mxcsr);
    }

    return 0;
}

static __inline__ int
fesetenv(const fenv_t *env)
{
	fenv_t __env = *env;
	int    mxcsr;

	__mxcsr = __get_mxcsr(__env);
	__ssesetmxcsr(__env, ~0);
	/*
	 * XXX Using fldenvx() instead of fldenv() tells the compiler that this
	 * instruction clobbers the i387 register stack.  This happens because
	 * we restore the tag word from the saved environment.  Normally, this
	 * would happen anyway and we wouldn't care, because the ABI allows
	 * function calls to clobber the i387 regs.  However, fesetenv() is
	 * inlined, so we need to be more careful.
	 */
	__fldenvx(__env);
	if (__HAS_SSE())
		__ldmxcsr(mxcsr);
	return (0);
}

int
fegetexceptflag(fexcept_t *except, int flags)
{
    fexcept_t exc;
#if (XMM)
    uint32_t  xexc;
#endif

    __asm__ ("fnstsw %0\n"
             "fwait\n"
             : "=m" (*&exc));
    *except = exc & flags & FE_ALL_EXCEPT;
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n"
                          "fwait\n"
                          : "=m" (*&xexc));
    *except |= xexc & flags & FE_ALL_EXCEPT;
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

int
fesetexceptflg(const fexcept_t *except, int flags)
{
    fenv_t   __env;
#if (XMM)
    uint32_t xexc;
#endif

    __asm__ __volatile__ ("fnstenv %0\n"
                          "fwait\n"
                          : "=m" (*&env));
    env.statword &= ~(flags & FE_ALL_EXCEPT);
    env.statword |= *except & flags & FE_ALL_EXCEPT;
    __asm__ __volatile__ ("fldenv %0\n" : : "m" (*&env));
#if (XMM)
    __asm__ __volatile__ ("stmxcsr %0\n" : "=m" (*&xexc));
    xexc &= ~(flags & FE_ALL_EXCEPT);
    xexc |= *except & flags & FE_ALL_EXCEPT;
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
