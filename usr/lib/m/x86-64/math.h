#ifndef __X86_64_MATH_H__
#define __X86_64_MATH_H__

#if defined(__x86_64__) || defined(__amd64__)
/* assume use of IEEE 64-bit double as long double */
#define __isnanl(x) isnan(x)
#endif /* __x86_64__ || __amd64__ */

#endif /* __X86_64_MATH_H__ */

