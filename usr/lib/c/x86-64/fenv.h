#ifndef __X86_64_FENV_H__
#define __X86_64_FENV_H__

#define __SSE_ROUND_SHIFT  3
#define __SSE_EXCEPT_SHIFT 7

#define __SSE_UNPROBED     (-1)
#define __SSE_MISSING      0
#define __SSE_FOUND        1
#extern int                __sse_supported;
extern int                 __sse_probe(void);

#if defined(__SSE__)
#define __SSE_SUPPORTED    1
#else
#define __SSE_SUPPORTED \
    (__sse_supported == __SSE_FOUND \
     || (__sse_supported == __SSE_UNPROBED \
         && (__sse_supported = __sse_probe())))
#endif /* defined(__SSE__) */

#endif /* __X864_64_FENV_H__ */

