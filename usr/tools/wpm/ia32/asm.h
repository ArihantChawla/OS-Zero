#ifndef __WPM_IA32_ASM_H__
#define __WPM_IA32_ASM_H__

#define asmnot(dest)                                                    \
    __asm__ __volatile__ ("not %0\n" : : "r" (dest))
#define asmand(src, dest)                                               \
    __asm__ __volatile__ ("and %0, %1\n" : : "r" (src), "r" (dest))
#define asmor(src, dest)                                                \
    __asm__ __volatile__ ("or %0, %1\n" : : "r" (src), "r" (dest))
#define asmxor(src, dest)                                               \
    __asm__ __volatile__ ("xor %0, %1\n" : : "r" (src), "r" (dest))
#define asmshl(cnt, dest)                                               \
    __asm__ __volatile__ ("shl %%cl, %1\n" : : "c" (cnt), "r" (dest))
#define asmshr(cnt, dest)                                               \
    __asm__ __volatile__ ("sar %%cl, %1\n" : : "c" (cnt), "r" (dest))
#define asmshrl(cnt, dest)                                              \
    __asm__ __volatile__ ("shr %%cl, %1\n" : : "c" (cnt), "r" (dest))
#define asmror(cnt, dest)                                               \
    __asm__ __volatile__ ("ror %%cl, %0\n" : : "c" (cnt), "r" (dest))
#define asmrol(cnt, dest)                                               \
    __asm__ __volatile__ ("rol %%cl, %0\n" : : "c" (cnt), "r" (dest))
#define asminc(dest)                                                    \
    __asm__ __volatile__ ("inc %0\n" : "=r" (dest) : "0" (dest))
#define asmdec(dest)                                                    \
    __asm__ __volatile__ ("dec %0\n" : "=r" (dest): "0" (dest))
#define asmadd(src, dest)                                               \
    __asm__ __volatile__ ("add %0, %1\n" : : "r" (src), "r" (dest))
#define asmsub(src, dest)                                               \
    __asm__ __volatile__ ("sub %0, %1\n" : : "r" (src), "r" (dest))
#define asmmul(src, dest)                                               \
    __asm__ __volatile__ ("imul %0, %1\n" : : "r" (src), "r" (dest))
#define asmdiv(src, dest)                                               \
    __asm__ __volatile__ ("idiv %0\n" : : "r" (src), "a" (dest))
#define asmmod(src, dest)                                               \
    __asm__ __volatile__ ("idiv %0\n" : : "r" (src), "a" (dest));       \
    __asm__ __volatile__ ("mov %%edx, %0\n" : "=r" (dest))

#endif /* __WPM_IA32_ASM_H__ */
