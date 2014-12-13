#ifndef __X86_64_FENV_H__
#define __X86_64_FENV_H__

#define __SSE_ROUND_SHIFT  3
#define __SSE_EXCEPT_SHIFT 7

extern int __sse_supported;
extern int __sse_probe(void);

static __inline__ int
feclearexcept(int mask)
{
    fenv_t env;
    
    if (mask == FE_ALL_EXCEPT) {
        __i387fnclex();
    } else {
        __i387fnstenv(&env.__x87);
        env.__x87.__status &= ~mask;
        __i387fldenv(env.__x87);
    }
    __ssestmxcsr(&env.__mxcsr);
    env.__mxcsr &= ~mask;
    __sseldmxcsr(env.__mxcsr);
    
    return 0;
}

static __inline__ int
fegetexceptflag(fexcept_t *except, int mask)
{
    int mxcsr;
    int status;
    
    __ssestmxcsr(&mxcsr);
    __i387fnstsw(&status);
    *except = (mxcsr | status) & mask;

    return 0;
}

static __inline__ int
fetestexcept(int mask)
{
    int mxcsr;
    int status;
    
    __ssestmxcsr(&mxcsr);
    __i387fnstsw(&status);
    status |= mxcsr;
    status &= mask;
    
    return status;
}

static __inline__ int
fesetround(int mode)
{
    int mxcsr;
    int ctrl;
    
    if (mode & ~__FE_ROUND_MASK) {

        return -1;
    }
    
    __i387fnstcw(&ctrl);
    ctrl &= ~__FE_ROUND_MASK;
    ctrl |= mode;
    __i387fldcw(ctrl);
    __ssestmxcsr(&mxcsr);
    mxcsr &= ~(__FE_ROUND_MASK << __SSE_ROUND_SHIFT);
    mxcsr |= mode << __SSE_ROUND_SHIFT;
    __sseldmxcsr(mxcsr);
    
    return 0;
}

static __inline__ int
fesetenv(const fenv_t *env)
{

    /* 
     * restoring tag word from saved environment clobbers i387 register stack;
     * the ABI allows function calls to do that, but we're inline so we need
     * to take care and use __i387fldenvx()
     */
    __i387fldenvx(env->__x87);
    __sseldmxcsr(env->__mxcsr);

    return 0;
}

#endif /* __X864_64_FENV_H__ */

