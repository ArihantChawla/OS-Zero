#ifndef __IEEE_754_H__
#define __IEEE_754_H__

#include <features.h>
#include <endian.h>

/* FIXME: add 128-bit unions and pick the right ones for long double :) */

#if defined(_ZERO_SOURCE) || defined(_GNU_SOURCE)

#if !defined(__FLOAT_WORD_ORDER)
#define __FLOAT_WORD_ORDER __BYTE_ORDER
#endif

#define IEEE754_FLOAT_BIAS          0x7f
#define IEEE754_DOUBLE_BIAS         0x3ff
#define IEEE754_LONG_DOUBLE80_BIAS  0x3fff
#define IEEE754_LONG_DOUBLE128_BIAS 0x7fff

#if defined(_GNU_SOURCE)
#define ieee754_float       __ieee754f
#define ieee754_double      __ieee754d
#define ieee754_long_double __ieee754ld80
#define val                 ieee
#define nan                 ieee_nan
#define mantissa            mant
#define mantissa0           mant0
#define mantissa1           mant1
#define exponent            exp
#define negative            sign
#define quiet_nan           quiet
#endif

#if (__BYTE_ORDER == __LITTLE_ENDIAN)

union __ieee754f {
    float f;
    struct {
        unsigned mant  : 23;
        unsigned exp   : 8;
        unsigned sign  : 1;
    } val;
    struct {
        unsigned mant  : 22;
        unsigned quiet : 1;
        unsigned exp   : 8;
        unsigned sign  : 1
    } nan;
};

union __ieee754ld128 {
    long double ld;
    struct {
        unsigned mant1 : 64;
        unsigned mant0 : 48;
        unsigned exp   : 15;
        unsigned sign  : 1;
    } val;
};

#if (__FLOAT_WORD_ORDER == __LITTLE_ENDIAN)

union __ieee754d {
    double d;
    struct {
        unsigned mant1 : 32;
        unsigned mant0 : 20;
        unsigned exp   : 11;
        unsigned sign  : 1;
    } val;
    struct {
        unsigned mant1 : 32;
        unsigned mant0 : 19;
        unsigned quiet : 1;
        unsigned exp   : 11;
        unsigned sign  : 1;
    } nan;
};

union __ieee754ld80 {
    long double ld;
    struct {
        unsigned mant1 : 32;
        unsigned mant0 : 32;
        unsigned exp   : 15;
        unsigned sign  : 1;
        unsigned _res  : 16;
    } val;
    struct {
        unsigned mant1 : 32;
        unsigned mant0 : 32;
        unsigned exp   : 15;
        unsigned sign  : 1;
        unsigned _res  : 16;
    } nan;
};

#elif (__FLOAT_WORD_ORDER == __BIG_ENDIAN)

union __ieee754d {
    double d;
    struct {
        unsigned mant0 : 20;
        unsigned exp   : 11;
        unsigned sign  : 1;
        unsigned mant1 : 32;
    } val;
    struct {
        unsigned mant0 : 19;
        unsigned quiet : 1;
        unsigned exp   : 11;
        unsigned sign  : 1;
        unsigned mant1 : 32;
    } nan;
};

union __ieee754ld80 {
    long double ld;
    struct {
        unsigned exp   : 15;
        unsigned sign  : 1;
        unsigned _res  : 16;
        unsigned mant0 : 32;
        unsigned mant1 : 32;
    } val;
    struct {
        unsigned exp   : 15;
        unsigned sign  : 1;
        unsigned _res  : 16;
        unsigned mant0 : 30;
        unsigned quiet : 1;
        unsigned one   : 1;
        unsigned mant1 : 32;
    } nan;
};

#endif /* __FLOAT_WORD_ORDER */

#elif (__BYTE_ORDER == __BIG_ENDIAN)

union __ieee754f {
    float f;
    struct {
        unsigned sign  : 1;
        unsigned exp   : 8;
        unsigned mant  : 23;
    } val;
    struct {
        unsigned sign  : 1;
        unsigned exp   : 8;
        unsigned quiet : 1;
        unsigned mant  : 22;
    } nan;
};

union __ieee754d {
    double d;
    struct {
        unsigned sign  : 1;
        unsigned exp   : 11;
        unsigned mant0 : 20;
        unsigned mant1 : 32;
    } val;
    struct {
        unsigned sign  : 1;
        unsigned exp   : 11;
        unsigned quiet : 1;
        unsigned mant0 : 19;
        unsigned mant1 : 32;
    } nan;
};

union __ieee754ld80 {
    long double ld;
    struct {
        unsigned sign  : 1;
        unsigned exp   : 15;
        unsigned _res  : 16;
        unsigned mant0 : 32;
        unsigned mant1 : 32;
    } val;
    struct {
        unsigned sign  : 1;
        unsigned exp   : 15;
        unsigned _res  : 16;
        unsigned one   : 1;
        unsigned quiet : 1;
        unsigned mant0 : 30;
        unsigned mant1 : 32;
    } nan;
};

union __ieee754ld128 {
    long double ld;
    struct {
        unsigned sign  : 1;
        unsigned exp   : 15;
        unsigned mant0 : 48;
        unsigned mant1 : 64;
    } val;
};

#endif /* __BYTE_ORDER */

#enif /* defined(_ZERO_SOURCE) || defined(_GNU_SOURCE) */

#endif /* __IEEE_754_H__ */

