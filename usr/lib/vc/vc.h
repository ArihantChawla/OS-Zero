/* zero libvc vcode interface library */

#ifndef __VC_VC_H__
#define __VC_VC_H__

#define VCOPNOP   0x00          // no operation done
#define VCOPADD   0x01          // dest += src
#define VCOPSUB   0x02          // dest -= src
#define VCOPMUL   0x03          // dest *= src
#define VCOPDIV   0x04          // dest /= src
#define VCOPMOD   0x05          // dest %= src
#define VCOPLT    0x06          // branch if src < dest
#define VCOPLTE   0x07          // branch if src <= dest
#define VCOPGT    0x08          // branch if src > dest
#define VCOPGTE   0x09          // branch if src >= dest
#define VCOPEQ    0x0a          // branch if src == dest
#define VCOPNEQ   0x0b          // branch if src != dest
#define VCOPSHL   0x0c          // left shift
#define VCOPSHR   0x0d          // logical right shift; fill with zero
#define VCOPSAR   0x0e          // arithmetic right shift; fill with sign-bit
#define VCOPNOT   0x0f          // bitwise negation; 2's complement
#define VCOPAND   0x10          // bitwise AND
#define VCOPOR    0x11          // bitwise OR
#define VCOPXOR   0x12          // bitwise exclusive-OR
#define VCOPSEL   0x13          // select
#define VCOPRAND  0x14
#define VCOPFLOOR 0x15
#define VCOPCEIL  0x16 
#define VCOPTRUNC 0x17
#define VCOPITOF  0x18
#define VCOPITOB  0x19
#define VCOPBTOI  0x1a
#define VCOPLOG   0x1b
#define VCOPSQRT  0x1c
#define VCOPEXP   0x1d
#define VCOPSINE  0x1e
#define VCOPCOS   0x1f
#define VCOPTAN   0x20
#define VCOPASIN  0x21
#define VCOPACOS  0x22
#define VCOPATAN  0x23
#define VCOPSINH  0x24
#define VCOPCOSH  0x25
#define VCOPTANH  0x26

#endif /* __VC_VC_H__ */

