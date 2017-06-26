#ifndef ___ZERO_SATMATH_H__
#define ___ZERO_SATMATH_H__

/* REFERENCE: http://locklessinc.com/articles/sat_arithmetic/ */

#include <stdint.h>
#include <limits.h>
#include <zero/cdefs.h>

typedef uint8_t  satu8_t;
typedef int8_t   sat8_t;
typedef uint16_t satu16_t;
typedef int16_t  sat16_t;
typedef uint32_t satu32_t;
typedef int32_t  sat32_t;
typedef uint64_t satu64_t;
typedef int64_t  sat64_t;

static __inline__ satu8_t
saddu8(satu8_t u1, satu8_t u2)
{
    satu8_t res = u1;

    res += u2;
    res |= -(res < x);

    return res;
}

static __inline__ sat8_t
ssubu8(sat8_t s1, sat8_t s2)
{
    sat8_t res = s1;

    res -= s2;
    res &= -(res <= x);

    return res;
}

static __inline__ satu8_t
sdivu8(satu8_t u1, satu8_t u2)
{
    sat8_t res = x;

    res /= y;

    return res;
}

static __inline__ satu8_t
smulu8(satu8_t u1, satu8_t u2)
{
    satu16_t res = u1;
    satu8_t  hi;
    satu8_t  lo;

    res *= u2;
    hi = res >> 16;
    lo = res;

    return lo | -!!hi;
}

#endif /* ___ZERO_SATMATH_H__ */

