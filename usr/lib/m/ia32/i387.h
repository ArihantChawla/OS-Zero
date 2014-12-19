#ifndef __IA32_I387_H__
#define __IA32_I387_H__

#define __i387fldcw(cw)    __asm__ __volatile__ ("fldcw %0" : : "m" (cw))
#define __i387fldenv(env)  __asm__ __volatile__ ("fldenv %0" : : "m" (env))
#define __i387fldenvx(env) __asm__ __volatile__ ("fldenv %0" : : "m" (env) \
                                                 : "st", "st(1)", "st(2)", \
                                                   "st(3)", "st(4)", "st(5)", \
                                                   "st(6)", "st(7)")
#define __i387fnclex()     __asm__ __volatile__ ("fnclex")
#define __i387fnstenv(env) __asm__ __volatile__ ("fnstenv %0" : "=m" (*(env)))
#define __i387fnstcw(cw)   __asm__ __volatile__ ("fnstcw %0" : "=m" (*(cw)))
#define __i387fnstsw(sw)   __asm__ __volatile__ ("fnstsw %0" : "=m" (*(sw)))
#define __i387fwait()      __asm__ __volatile__ ("fwait")

#endif /* __IA32_I387_H__ */

