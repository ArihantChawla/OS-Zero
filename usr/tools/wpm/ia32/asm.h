#ifndef __WPM_IA32_ASM_H__
#define __WPM_IA32_ASM_H__

#define asmnot(dest)                                                    \
    __asm__ __volatile__ ("not %0\n" : "=r" (dest) : "0" (dest))
#define asmand(src, dest)                                               \
    __asm__ __volatile__ ("and %1, %0\n" : "=r" (dest) : "r" (src), "0" (dest))
#define asmor(src, dest)                                                \
    __asm__ __volatile__ ("or %1, %0\n" : "=r" (dest) : "r" (src), "0" (dest))
#define asmxor(src, dest)                                               \
    __asm__ __volatile__ ("xor %1, %0\n" : "=r" (dest) : "r" (src), "0" (dest))
#define asmshl(cnt, dest)                                               \
    __asm__ __volatile__ ("shl %%cl, %0\n"                              \
                          : "=r" (dest) : "c" (cnt), "0" (dest))
#define asmshr(cnt, dest)                                               \
    __asm__ __volatile__ ("sar %%cl, %0\n"                              \
                          : "=r" (dest) : "c" (cnt), "0" (dest))
#define asmshrl(cnt, dest)                                              \
    __asm__ __volatile__ ("shr %%cl, %0\n"                              \
                          : "=r" (dest) : "c" (cnt), "0" (dest))
#define asmror(cnt, dest)                                               \
    __asm__ __volatile__ ("ror %%cl, %0\n"                              \
                          : "=r" (dest) : "c" (cnt), "0" (dest))
#define asmrol(cnt, dest)                                               \
    __asm__ __volatile__ ("rol %%cl, %0\n"                              \
                          : "=r" (dest) : "c" (cnt), "0" (dest))
#define asminc(dest)                                                    \
    __asm__ __volatile__ ("inc %0\n" : "=r" (dest) : "0" (dest))
#define asmdec(dest)                                                    \
    __asm__ __volatile__ ("dec %0\n" : "=r" (dest): "0" (dest))
#define asmadd(src, dest)                                               \
    __asm__ __volatile__ ("add %1, %0\n"                                \
                          : "=r" (dest) : "r" (src), "0" (dest))
#define asmsub(src, dest)                                               \
    __asm__ __volatile__ ("sub %1, %0\n"                                \
                          : "=r" (dest) : "r" (src), "0" (dest))
#define asmmul(src, dest)                                               \
    __asm__ __volatile__ ("imul %1, %0\n"                               \
                          : "=r" (dest) : "r" (src), "0" (dest))
#define asmdiv(src, dest)                                               \
    __asm__ __volatile__ ("idiv %0\n" : "=a" (dest) : "r" (src), "0" (dest))
#define asmmod(src, dest)                                               \
    __asm__ __volatile__ ("idiv %0\n" : "=r" (dest) : "r" (src), "0" (dest)); \
    __asm__ __volatile__ ("mov %%edx, %0\n" : "=r" (dest))

#endif /* __WPM_IA32_ASM_H__ */
