#ifndef __IA32_MATH_H__
#define __IA32_MATH_H__

#include <x86-64/math.h>

/* unsigned [integer] bit masks for IEEE-754 80-bit long double */
#if !defined(__LDBL_SIGNBIT)
#define __LDBL_SIGNBIT  0x00008000
#define __LDBL_EXPBITS  0x00007fff
#define __LDBL_MANTBITS UINT64_C(0xffffffffffffffff)
#endif

/* IEEE 80-bit floating point format */
#if !defined(__isnanl)
#define __isinfl(x)                                                     \
    ((((*(uint32_t *)&(x)[2]) & __LDBL_EXPBITS) == __LDBL_EXPBITS)      \
     && (!(*(uint64_t *)&(x))))
#define __isnanl(x)                                                     \
    ((((((uint32_t *)&(x))[2]) & __LDBL_EXPBITS) == __LDBL_EXPBITS)     \
     && ((uint64_t *)&(x)))
#define __issignalingl(x)                                               \
    (__isnanl(x) && !((*(uint64_t *)&(x)) & UINT64_C(0x8000000000000000)))
#endif
/*
 * i387 assembly operations
 * - assume default rounding mode; no messing around with it
 */

#define __fpusqrt(x, ret)                                               \
    __asm__ __volatile__ ("fldl %0\n" : : "m" (x));                     \
    __asm__ __volatile__ ("fsqrt\n");                                   \
    __asm__ __volatile__ ("fstpl %0\n"                                  \
                          "fwait\n"                                     \
                          : "=m" (ret))

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

