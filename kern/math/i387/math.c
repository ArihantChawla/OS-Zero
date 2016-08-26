#include <stdint.h>

double
sin(double x)
{
    double retval;

#if 0
    if (isnan(x)) {
        retval = x;
    } else if (fpclassify(x) == FP_INFINITE) {
        errno = EDOM;
        feraiseexcept(FE_INVALID);
        if (dgetsign(x)) {
            retval = getsnan(x);
        } else {
            retval = getnan(x);
        }
    } else {
#endif
        __asm__ __volatile__ ("fldl %0\n" : : "m" (x));
        __asm__ __volatile__ ("fsin\n");
        __asm__ __volatile__ ("fstpl %0\n"
                              "fwait\n"
                              : "=m" (retval));
#if 0
    }
#endif
    
    return retval;
}

double
ceil(double d)
{
    double   ret;
    uint32_t fcw;
    uint32_t tmp;
    
    __asm__ __volatile__ ("fstcw %0\n"
                          "movw %0, %%dx\n"
                          "orw $0x0800, %%dx\n"
                          "andw $0xfbff, %%dx\n"
                          "movw %%dx, %1\n"
                          "fldcw %1\n"
                          "fldl %3\n"
                          "frndint\n"
                          "fstpl %2\n"
                          "fldcw %0\n"
                          "fwait\n"
                          : "=m" (fcw), "=m" (tmp), "=m" (ret)
                          : "m" (d)
                          : "edx");
    
    return ret;
}

