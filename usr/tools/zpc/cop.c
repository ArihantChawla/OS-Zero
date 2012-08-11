#include <stdint.h>
#include <stdio.h>
#include <zpc/zpc.h>
#include <zpc/op.h>

int64_t
not64(int64_t arg1, int64_t dummy)
{
    int64_t src = arg1;
    int64_t dest;

    dest = ~src;

    return dest;
}

int64_t
and64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest &= src;

    return dest;
}

int64_t
or64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest |= src;

    return dest;
}

int64_t
xor64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest ^= src;

    return dest;
}

int64_t
shl64(int64_t arg1, int64_t arg2)
{
    int64_t cnt = arg1;
    int64_t dest = arg2;

    dest <<= cnt;

    return dest;
}

int64_t
shr64(int64_t arg1, int64_t arg2)
{
    int64_t cnt = arg1;
    int64_t dest = arg2;
    int64_t sign = dest & INT64_C(0x8000000000000000);

    sign = (sign & INT64_C(0xffffffffffffffff)) << (64 - cnt);
    dest >>= cnt;
    dest |= sign;

    return dest;
}

int64_t
shrl64(int64_t arg1, int64_t arg2)
{
    int64_t cnt = arg1;
    int64_t dest = arg2;
    int64_t mask = INT64_C(0xffffffffffffffff) >> cnt;

    dest >>= cnt;
    dest &= mask;

    return dest;
}

int64_t
ror64(int64_t arg1, int64_t arg2)
{
    int64_t cnt = arg1;
    int64_t dest = arg2;
    int64_t mask = INT64_C(0xffffffffffffffff) >> (64 - cnt);
    int64_t bits = dest & mask;

    bits <<= 64 - cnt;
    dest >>= cnt;
    dest |= bits;

    return dest;
}

int64_t
rol64(int64_t arg1, int64_t arg2)
{
    int64_t cnt = arg1;
    int64_t dest = arg2;
    int64_t mask = INT64_C(0xffffffffffffffff) << (64 - cnt);
    int64_t bits = dest & mask;

    bits >>= 64 - cnt;
    dest <<= cnt;
    dest |= bits;

    return dest;
}

int64_t
inc64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;

    src++;

    return src;
}

int64_t
dec64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;

    src--;

    return src;
}

int64_t
add64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest += src;

    return dest;
}

int64_t
sub64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest -= src;

    return dest;
}

int64_t
mul64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest *= src;

    return dest;
}

int64_t
div64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest /= src;

    return dest;
}

int64_t
mod64(int64_t arg1, int64_t arg2)
{
    int64_t src = arg1;
    int64_t dest = arg2;

    dest %= src;

    return dest;
}

