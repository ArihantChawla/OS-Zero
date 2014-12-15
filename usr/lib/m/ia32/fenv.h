#ifndef __IA32_FENV_H__
#define __IA32_FENV_H__

#include <stdint.h>
#include <ia32/i387.h>
#include <ia32/sse.h>

typedef uint16_t      fexcept_t;

#define FE_TONEAREST  0x0000
#define FE_TOWARDZERO 0x0400
#define FE_UPWARD     0x0800
#define FE_DOWNWARD   0x0c00
#define __FE_ROUND_MASK                                                 \
    (FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO)

#if !defined(__x86_64__) && !defined(__amd64__)

/* FPU environment for i387 */
#define __fegetmxcsr(env) (((env).__mxcsrhi << 16) | ((env).__mxcsrlo))
#define __fesetmxcsr(env, u32)                                          \
    do {                                                                \
        (env).__mxcsrhi = (uint32_t)(u32) >> 16;                        \
        (env).__mxcsrlo = (uint16_t)(u32);                              \
    } while (0)
typedef struct {
    uint16_t __ctrl;
    uint16_t __mxcsrhi;
    uint16_t __status;
    uint16_t __mxcsrlo;
    uint32_t __tag;
    uint8_t  __other[16];
} fenv_t;

#endif /* not 64-bit */

#if defined(__x86_64__) || defined(__amd64__)
#include <x86-64/fenv.h>
#endif

#if defined(__SSE__)
#define    __sse_online() 1
#else /* defined(!__SSE__) */
extern int __sse_supported;
extern int __sse_probe(void);
#define    __SSE_UNPROBED (-1)
#define    __SSE_MISSING  0
#define    __SSE_FOUND    1
#define    __sse_online()                                               \
    (__sse_supported == __SSE_FOUND                                     \
     || (__sse_supported == __SSE_UNPROBED                              \
         && (__sse_supported = __sse_probe())))
#endif /* defined(__SSE__) */

/* 64-bit architectures have these in <x86-64/fenv.h> which we included */
#if !defined(__x86_64__) && !defined(__amd64__)

static __inline__ int
feclearexcept(int mask)
{
    fenv_t env;
    int    mxcsr;
    
    if (mask & FE_ALL_EXCEPT) {
        __i387fnclex();
    } else {
        __i387fnstenv(&env);
        env.__status &= ~mask;
        __i387fldenv(env);
    }
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
        mxcsr &= ~mask;
        __sseldmxcsr(mxcsr);
    }

    return 0;
}

static __inline__ int
fegetexceptflag(fexcept_t *except, int mask)
{
    int mxcsr;
    int status;
    
    __i387fnstsw(&status);
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
    } else {
        mxcsr = 0;
    }
    *except = (mxcsr | status) & mask;

    return 0;
}

static __inline__ int
fetestexcept(int mask)
{
    int mxcsr;
    int status;

    __i387fnstsw(&status);
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
    } else {
        mxcsr = 0;
    }
    status = (mxcsr & mask);
    
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
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
        mxcsr &= ~(__FE_ROUND_MASK << __SSE_ROUND_SHIFT);
        mxcsr |= mode << __SSE_ROUND_SHIFT;
        __sseldmxcsr(mxcsr);
    }
    
    return (0);
}

static __inline__ int
fesetenv(const fenv_t *env)
{
    fenv_t fenv = *env;
    int    mxcsr;
        
    mxcsr = __fegetmxcsr(fenv);
    __fesetmxcsr(fenv, 0xffffffff);
    /* 
     * restoring tag word from saved environment clobbers i387 register stack;
     * the ABI allows function calls to do that, but we're inline so we need
     * to take care and use __i387fldenvx()
     */
    __i387fldenvx(fenv);
    if (__sse_online()) {
        __sseldmxcsr(mxcsr);
    }
    
    return 0;
}

#endif /* not 64-bit */

extern int fesetexceptflag(const fexcept_t *except, int mask);

static __inline__ int
feraiseexcept(int mask)
{
    fexcept_t except = mask;
    
    fesetexceptflag(&except, mask);
    __i387fwait();

    return 0;
}

static __inline__ int
fegetround(void)
{
    int ctrl;

    __i387fnstcw(&ctrl);
    ctrl &= __FE_ROUND_MASK;
    
    return ctrl;
}

#if defined(USEBSD) && (USEBSD)

static __inline__ int
fegetexcept(void)
{
    int ctrl;

    __i387fnstsw(&ctrl);
    ctrl = ~ctrl & FE_ALL_EXCEPT;
    
    return ctrl;
}

#endif /* USEBSD */

#endif /* __IA32_FENV_H__ */

