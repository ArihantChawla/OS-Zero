#ifndef __IA32_FENV_H__
#define __IA32_FENV_H__

#include <stdint.h>
#if defined(__x86_64__) || defined(__amd64__) \
#include <x86-64/fenv.h>
#endif

#define FE_TONEAREST  0x0000
#define FE_TOWARDZERO 0x0400
#define FE_UPWARD     0x0800
#define FE_DOWNWARD   0x0c00
#define __ROUND_MASK  (FE_TONEAREST | FE_DOWNWARD | FE_UPWARD | FE_TOWARDZERO)

typedef uint16_t      fexcept_t;

/* FPU environment for i387 */
#define __i387_getmxcsr(env) (((env).__mxcsr_hi << 16) | ((env).__mxcsrlo))
typedef struct {
    uint16_t __ctrl;
    uint16_t __mxcsrhi;
    uint16_t __status;
    uint16_t __mxcsrlo;
    uint32_t __tag;
    uint8_t  __other[16];
} fenv_t;

#define __i387_fldcw(__cw)    __asm__ __volatile__ ("fldcw %0" : : "m" (__cw))
#define __i387_fldenv(__env)  __asm__ __volatile__ ("fldenv %0" : : "m" (__env))
#define __i387_fnclex()       __asm__ __volatile__ ("fnclex")
#define __i387_fnstenv(__env) __asm__ __volatile__ ("fnstenv %0" : "=m" (*(__env)))
#define __i387_fnstcw(__cw)   __asm__ __volatile__ ("fnstcw %0" : "=m" (*(__cw)))
#define __i387_fnstsw(__sw)   __asm__ __volatile__ ("fnstsw %0" : "=am" (*(__sw)))
#define __i387_fwait()        __asm__ __volatile__ ("fwait")
#define __i387_ldmxcsr(__csr) __asm__ __volatile__ ("ldmxcsr %0" : : "m" (__csr))
#define __i387_stmxcsr(__csr) __asm__ __volatile__ ("stmxcsr %0" : "=m" (*(__csr)))

/* TODO: __inline__ this */
#define feclearexcept(mask) \
    (do {
        fenv_t __env;
        int    __mxcsr;

        if (mask & FE_ALL_EXCEPT) {
            __i387_fnclex();
        } else {
            __i387_fnstenv(&__env);
            __env.__status &= ~mask;
            __i387_fldenv(__env);
        }
        if (__SSE_SUPPORTED) {
            __i387_stmxcsr(&__mxcsr);
            __mxcsr &= ~mask;
            __i387_ldmxcsr(__mxcsr);
        }
    } while (0), 0)

#endif /* __IA32_FENV_H__ */

