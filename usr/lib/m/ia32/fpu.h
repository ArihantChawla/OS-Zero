#ifndef __M_IA32_FPU_H__
#define __M_IA32_FPU_H__

#define __i387fldcw(__cw)   __asm__ __volatile__ ("fldcw %0" : : "m" (__cw))
#define __i387fldenv(env)   __asm__ __volatile__ ("fldenv %0" : : "m" (env))
#define __i387fnclex()      __asm__ __volatile__ ("fnclex")
#define __i387fnstenv(env)  __asm__ __volatile__ ("fnstenv %0" : "=m" (*(env)))
#define __i387fnstcw(__cw)  __asm__ __volatile__ ("fnstcw %0" : "=m" (*(__cw)))
#define __i387fnstsw(__sw)  __asm__ __volatile__ ("fnstsw %0" : "=am" (*(__sw)))
#define __i387fwait()       __asm__ __volatile__ ("fwait")
#define __sseldmxcsr(__csr) __asm__ __volatile__ ("ldmxcsr %0" : : "m" (__csr))
#define __ssestmxcsr(__csr) __asm__ __volatile__ ("stmxcsr %0" : "=m" (*(__csr)))

#endif /* __M_IA32_FPU_H__ */

