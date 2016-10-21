#ifndef __X86_64_MATH_H__
#define __X86_64_MATH_H__

#if defined(__x86_64__) || defined(__amd64__)
#if (defined(__SSE__) || defined(__AVX__)                               \
     || (defined(_M_IX86_FP) && _M_IX86_FP >= 1))
#define USEIEEEQUAD128 1
#else
/* assume use of IEEE 64-bit double as long double */
#define __isnanl(x) isnan(x)
#endif
#endif /* __x86_64__ || __amd64__ */

#endif /* __X86_64_MATH_H__ */

