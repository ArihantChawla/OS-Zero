#include <stdint.h>
#include <fenv.h>
#include <zero/cdecl.h>

int
fesetexceptflag(const fexcept_t *except, int mask)
{
    fenv_t env;
    int    mxcsr;
    
    __fnstenv(&env);
    env.__status &= ~mask;
    env.__status |= *except & mask;
    __i387fldenv(env);
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
        mxcsr &= ~mask;
        mxcsr |= *mask & excepts;
        __sseldmxcsr(mxcsr);
    }
    
    return 0;
}

int
feholdexcept(fenv_t *env)
{
    int mxcsr;
    
    __i387fnstenv(env);
    __i387fnclex();
    if (__sse_online()) {
        __ssesetmxcsr(&mxcsr);
        __ssesetmxcsr(*env, mxcsr);
        mxcsr &= ~FE_ALL_EXCEPT;
        mxcsr |= FE_ALL_EXCEPT << _SSE_EMASK_SHIFT;
        __i387ldmxcsr(mxcsr);
    }

    return 0;
}

#if !defined(__x86_64__) && !defined(__amd64__)

int
fesetround(int mode)
{
    int mxcsr;
    int ctrl;

    if (mode & ~__FE_ROUND_MASK) {

        return -1;
    }

    __i387fnstcw(&ctrl);
    ctrl &= ~_ROUND_MASK;
    ctrl |= mode;
    __i387fldcw(ctrl);
    
    if (__sse_online()) {
        __ssesetmxcsr(&mxcsr);
        mxcsr &= ~(_ROUND_MASK << _SSE_ROUND_SHIFT);
        mxcsr |= mode << _SSE_ROUND_SHIFT;
        __sseldmxcsr(mxcsr);
    }

    return 0;
}

#endif /* not 64-bit */

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

int
fesetenv(const fenv_t *env)
{
    fenv_t fenv = *env;
    int    mxcsr;
    
    __mxcsr = __get_mxcsr(fenv);
    __ssesetmxcsr(fenv, ~0);
    /* 
     * restoring tag word from saved environment clobbers i387 register stack;
     * the ABI allows function calls to do that, but we're inline so we need
     * to take care and use __i387fldenvx()
     */
    __i387fldenvx(__env);
    if (__sse_online()) {
        __sseldmxcsr(mxcsr);
    }

    return 0;
}
int
feupdateenv(const fenv_t *env)
{
    int mxcsr;
    int status;
    
    __i387fnstsw(&status);
    if (__sse_online()) {
        __ssesetmxcsr(&mxcsr);
    } else {
        mxcsr = 0;
    }
    fesetenv(env);
    feraiseexcept((mxcsr | status) & FE_ALL_EXCEPT);

    return 0;
}

/* non-POSIX.1 functions */

int
fedisableexcept(int mask)
{
    int mxcsr;
    int oldmask;
    int control;
    
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
    int oldmask;
    int mxcsr;
    int status;
    int ctrl;

    mask &= FE_ALL_EXCEPT;
    __i387fnstsw(&__control;);
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
    } else {
        __mxcsr = 0;
    }
    mxcsr >>= __SSE_EXCEPT_SHIFT;
    oldmask = ~(ctrl | mxcsr) & FE_ALL_EXCEPT;
    ctrl &= ~mask;
    __i387fldcw(ctlr);
    if (__sse_online()) {
        mxcsr &= ~(mask << __SSE_EXCEPT_SHIFT);
        __sseldmxcsr(mxcsr);
    }

    return oldmask;
}

