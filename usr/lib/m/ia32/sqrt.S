/*
 * implementation of sqrt() for the i387 as written by R on stackoverflow.com :)
 */

 .globl	__fpusqrt

 __fpusqrt:
    fldl 4(%esp)
    fsqrt
    fstsw %ax
    subl $12,%esp
    fld %st(0)
    fstpt (%esp)
    movl (%esp),%ecx
    andl $0x7ff,%ecx
    cmpl $0x400,%ecx
    jnz 1f
    andl $0x200,%eax
    subl $0x100,%eax
    subl %eax,(%esp)
    fstp %st(0)
    fldt (%esp)
#if 0
1:  add $12,%esp
    fstpl 4(%esp)
    fldl 4(%esp)
#endif
1:
    fstpl -8(%esp)
    fldl -8(%esp)
    addl $12, %esp
    ret

