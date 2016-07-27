#ifndef __ZERO_X86_64_BIGNUM_H__
#define __ZERO_X86_64_BIGNUM_H__

typedef __int128_t  i128;
typedef __uint128_t u128;

u128 addu128(u128 a, u128 b);
u128 mulu128(u128 a, u128 b);

static __inline__ i128
add128(i128 a, i128 b) {

    return a + b;
}

static __inline__ i128
sub128(i128 a, i128 b) {

    return a - b;
}

static __inline__ i128
mul128(i128 a, i128 b) {

    return a * b;
}

static __inline__ i128
div128(i128 a, i128 b) {

    return a / b;
}

static __inline__ i128
rem128(i128 a, i128 b) {

    return a % b;
}

#endif /* __ZERO_X86_64_BIGNUM_H__ */

