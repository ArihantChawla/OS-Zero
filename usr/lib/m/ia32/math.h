#ifndef __IA32_MATH_H__
#define __IA32_MATH_H__

#include <x86-64/math.h>

/* IEEE 80-bit floating point format */
#if !defined(isnanl)
#define isnanl(x)                                                       \
    (*(uint64_t *)&(x) == UINT64_C(0xffffffffffffffff)                  \
     && (((uint32_t *)&(x)[2] & 0x7fff) == 0x7fff))
#endif

#if 0
#define __fpusqrt(x, ret)                                               \
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsqrt\n");                                   \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))
#endif /* 0 */

/* FIXME: R's code from stackoverflow.com :) rewritten into inline assembly */
/* - finish this one */
#define __fpusqrt(x, ret) \
    do { \
        int16_t __status; \
        int32_t __tmp1;
        int32_t __tmp2;
        int32_t __res;
        \
        __asm__ __volatile__ ("fldl %4\n" \        // load x (as double)
                              "fsqrt\n" \          // sqrt(x)
                              "fstsw %1\n" \       // __status <- FPU status
                              "subl $20, %esp\n"   // %esp -> x
                              "fld st(0)\n"        // load x into st(0)
                              "fstpt (%esp)\n"     // x = sqrt(x)
                              "movl (%esp), %2\n"  // load square root in __tmp1
                              "movl (%esp), %3\n"  // load square root in __tmp2
                              "andl $0x07ff, %2\n" // __tmp1 &= 0x7ff
                              "cmpl $0x0400, %2\n" // compare __tmp1 with 0x400
                              "jnz 1f\n"           // jump to 1: if equal
                              "andl $0x0200, %1\n" // __status &= 0x200
                              "subl $0x0100, %1\n" // __status -= 0x100
                              "subl %1, %3\n"      // __tmp2 -= __status
                              "fstp st(0)\n"       // x = sqrt(x)
                              "fldt (%esp)\n"      // load sqrt(x) as double
                              "1:\n"
                              "fstp 20(%esp)\n"    // store sqrt(x) as double
                              "fldl 20(%esp)\n"    // load sqrt(x) as double
                              "addl $16, %esp\n"   // adjust stack pointer
                              "retl\n"             // return
                              : "=r" (ret), "=r" (__status),
                                "=r" (__tmp1), "=r" (__tmp2)
                              : "rm" (x)
                              :)
    } while (0)

#define __fpusin(x, ret)                                                \
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsin\n");                                    \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))
#define __fpucos(x, ret)                                                \
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fcos\n");                                    \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))
#define __fputan(x, ret)                                                \
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fptan\n");                                   \
    __asm__ __volatile__ ("fstpl %0\n" : "=m" (tmp));                   \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))
#define __fpuatan(x, ret)                                               \
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fpatan\n");                                  \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))

#define __fpusinf(x, ret)                                               \
    __asm__ __volatile__ ("flds %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsin\n");                                    \
    __asm__ __volatile__ ("fstps %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))
#define __fpucosf(x, ret)                                              \
    __asm__ __volatile__ ("flds %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fcos\n");                                    \
    __asm__ __volatile__ ("fstps %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))

#define __fpusinl(x, ret)                                               \
    __asm__ __volatile__ ("fldt %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsin\n");                                    \
    __asm__ __volatile__ ("fstpt %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret));
#define __fpucosl(x, ret)                                               \
    __asm__ __volatile__ ("fldt %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fcos\n");                                    \
    __asm__ __volatile__ ("fstpt %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))

#if defined(_GNU_SOURCE)
#define __fpusincos(x, sin, cos)                                        \
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsincos\n");                                 \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (*cos));                               \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (*sin))
#define __fpusincosf(x, sin, cos)                                       \
    __asm__ __volatile__ ("flds %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsincos\n");                                 \
    __asm__ __volatile__ ("fstps %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (*cos));                               \
    __asm__ __volatile__ ("fstps %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (*sin))
#define __fpusincosl(x, sin, cos)                                       \
    __asm__ __volatile__ ("fldt %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsincos\n");                                 \
    __asm__ __volatile__ ("fstpt %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (*cos));                               \
    __asm__ __volatile__ ("fstpt %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (*sin));
#endif /* _GNU_SOURCE */

#endif /* __IA32_MATH_H__ */

