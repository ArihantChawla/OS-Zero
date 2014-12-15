#ifndef __IA32_SSE_H__
#define __IA32_SSE_H__

#define __sseldmxcsr(csr)  __asm__ __volatile__ ("ldmxcsr %0" : : "m" (csr))
#define __ssestmxcsr(csr)  __asm__ __volatile__ ("stmxcsr %0" : "=m" (*(csr)))

#endif /* __IA32_SSE_H__ */

