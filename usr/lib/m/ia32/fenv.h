#ifndef __IA32_FENV_H__
#define __IA32_FENV_H__

#include <stdint.h>
typedef uint16_t fexcept_t;

#define FE_TONEAREST  0x0000
#define FE_TOWARDZERO 0x0400
#define FE_UPWARD     0x0800
#define FE_DOWNWARD   0x0c00
#define __FE_ROUND_MASK                                                 \
    (FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO)

#if !defined(__x86_64__) && !defined(__amd64__)

/* FPU environment for i387 */
#define __fegetmxcsr(env) (((env).__mxcsr_hi << 16) | ((env).__mxcsrlo))
#define __fesetmxcsr(env, u32)                                          \
    do {                                                                \
        (env).__mxcsr_hi = (uint32_t)(x) >> 16;                         \
        (env).__mxcsr_lo = (uint16_t)(x);                               \
    } while (0)
typedef struct {
    uint16_t __ctrl;
    uint16_t __mxcsrhi;
    uint16_t __status;
    uint16_t __mxcsrlo;
    uint32_t __tag;
    uint8_t  __other[16];
} fenv_t;

#else /* not 64-bit */

typedef struct {
    struct {
        uint32_t __ctrl;
        uint32_t __status;
        uint32_t __tag;
        uint8_t __other[16];
    } __x87;
    uint32_t __mxcsr;
} fenv_t;

#endif

#if defined(__x86_64__) || defined(__amd64__)
#include <x86-64/fenv.h>
#endif

#define __SSE_UNPROBED (-1)
#define __SSE_MISSING  0
#define __SSE_FOUND    1
#if defined(__SSE__)
#define __sse_online() 1
#else /* defined(!__SSE__) */
#define __sse_online()                                                  \
    (__sse_supported == __SSE_FOUND                                     \
     || (__sse_supported == __SSE_UNPROBED                              \
         && (__sse_supported = __sse_probe())))
#endif /* defined(__SSE__) */

static __inline__ int
fesetexceptflag(const fexcept_t *except, int mask)
{
    fenv_t env;
    int    mxcsr;
    
    __i387fnstenv(&env);
#if defined(__x86_64__) || defined(__amd64__)
    env.__x87.__status &= ~mask;
    env.__x87.__status |= *except & mask;
#else
    env.__status &= ~mask;
    env.__status |= *except & mask;
#endif
    __i387fldenv(env);
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
        mxcsr &= ~mask;
        mxcsr |= *except & mask;
        __sseldmxcsr(mxcsr);
    }
    
    return 0;
}

/* 64-bit architectures have these in <x86-64/fenv.h> which we included */
#if !defined(__x86_64__) && !defined(__amd64__)

static __inline__ int
feclearexcept(int mask)
{
    fenv_t   env;
    uint32_t mxcsr;
    
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
    uint32_t mxcsr;
    int     status;
    
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
    status |= mxcsr;
    status &= mask;
    
    return status;
}

static __inline__ int
fesetround(int mode)
{
    int mxcsr;
    int ctrl;
    
    if (__round & ~__FE_ROUND_MASK) {

        return -1;
    }
    __fnstcw(&ctrl);
    ctrl &= ~__FE_ROUND_MASK;
    ctrl |= mode;
    __fldcw(ctrl);
    if (__sse_online()) {
        __i387stmxcsr(&mxcsr);
        mxcsr &= ~(_ROUND_MASK << _SSE_ROUND_SHIFT);
        mxcsr |= __round << _SSE_ROUND_SHIFT;
        __sseldmxcsr(mxcsr);
    }
    
    return (0);
}

static __inline__ int
fesetenv(const fenv_t *env)
{
    fenv_t env = *env;
    int    mxcsr;
        
    mxcsr = __fegetmxcsr(env);
    __fesetmxcsr(env, ~0);
    __i387fldenv(env);
    if (__sse_online()) {
        __sseldmxcsr(mxcsr);
    }
    
    return 0;
}

#endif /* not 64-bit */

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

static __inline__ int
fegetexcept(void)
{
    uint32_t mxcsr;
    int      status;

    __i387fnstsw(&status);
    if (__sse_online()) {
        __ssestmxcsr(&mxcsr);
    } else {
        mxcsr = 0;
    }
    
    return 0;
}

#endif /* __IA32_FENV_H__ */

