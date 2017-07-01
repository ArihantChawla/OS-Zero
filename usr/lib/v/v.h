/* zero libvc vcode interface library */

#ifndef __VC_VC_H__
#define __VC_VC_H__

#include <stdint.h>

#define V_NOP   0x00    // no operation done
#define V_NOT   0x01    // bitwise negation; 2's complement
#define V_AND   0x02    // bitwise AND
#define V_OR    0x03    // bitwise OR
#define V_XOR   0x04    // bitwise exclusive-OR
#define V_SHL   0x05    // left shift
#define V_SHR   0x06    // logical right shift; fill with zero
#define V_SAR   0x07    // arithmetic right shift; fill with sign-bit
#define V_INC   0x08    // arg1++;
#define V_DEC   0x09    // arg1--;
#define V_ADD   0x0a    // arg2 += arg1;
#define V_SUB   0x0b    // arg2 -= arg1;
#define V_MUL   0x0c    // arg2 *= arg1;
#define V_DIV   0x0d    // arg2 /= arg1;
#define V_MOD   0x0e    // arg2 %= arg1;
#define V_LT    0x0f    // arg1 < arg2;
#define V_LTE   0x10    // arg1 <= arg2;
#define V_GT    0x11    // arg1 > arg2;
#define V_GTE   0x12    // arg1 >= arg2;
#define V_EQ    0x13    // arg1 == arg2;
#define V_NEQ   0x14    // arg1 != arg2;
#define V_SEL   0x15    // select
#define V_RAND  0x16    // arg2 = (c) ? arg1 : arg2;
#define V_FLOOR 0x17
#define V_CEIL  0x18 
#define V_TRUNC 0x19
#define V_ITOF  0x1a
#define V_ITOB  0x1b
#define V_BTOI  0x1c
#define V_LOG   0x1d
#define V_SQRT  0x1f
#define V_EXP   0x1f
#define V_SINE  0x20
#define V_COS   0x21
#define V_TAN   0x22
#define V_ASIN  0x23
#define V_ACOS  0x24
#define V_ATAN  0x25
#define V_SINH  0x26
#define V_COSH  0x27
#define V_TANH  0x28

struct viarg {
    union {
#if (V_WORDSIZE == 64)
        int64_t  i64;
        uint64_t u64;
#endif
        int32_t  i32;
        uint32_t u32;
        int32_t  i16;
        uint32_t u16;
        int32_t  i8;
        uint32_t u8;
    } val;
    size_t       size;
};

struct vfarg {
    union {
        double d64;
        float  f32;
    } val;
    size_t     size;
};

struct varg {
    union {
        struct viarg i;
        struct vfarg f;
    } val;
    unsigned long flg;
};

#define V_BOOLEAN_BIT   (1UL << 0)
#define V_FLOAT_BIT     (1UL << 1)
#define V_COMPLEX_BIT   (1UL << 2)
#define V_VECTOR_BIT    (1UL << 25)
#define V_VOLATILE_BIT  (1UL << 26)
#define V_STATIC_BIT    (1UL << 27)
#define V_REGISTER_BIT  (1UL << 28)
#define V_AUTOMATIC_BIT (1UL << 29)
#define V_CONST_BIT     (1UL << 30)
#define V_LITERAL_BIT   (1UL << 31)
struct viop {
    struct varg   arg1;
    struct varg   arg2;
    unsigned long flg;
#if (V_WORDBITS == 64)
    uint64_t      inst;
#else
    uint32_t      inst;
#endif
    size_t        size;
};

#endif /* __VC_VC_H__ */

