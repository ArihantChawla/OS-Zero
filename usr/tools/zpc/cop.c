#include <wpc/op.h>

void
not32(void *arg1, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest;

    dest = ~src;
    wpcsetval64(ret, dest);

    return;
}

void
not64(void *arg1, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest;

    dest = ~src;
    wpcsetval64(ret, dest);

    return;
}

void
and32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest &= src;
    wpcsetval64(ret, dest);

    return;
}

void
and64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest &= src;
    wpcsetval64(ret, dest);

    return;
}

void
or32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest |= src;
    wpcsetval64(ret, dest);

    return;
}

void
or64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest |= src;
    wpcsetval64(ret, dest);

    return;
}

void
xor32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest ^= src;
    wpcsetval64(ret, dest);

    return;
}

void
xor64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest ^= src;
    wpcsetval64(ret, dest);

    return;
}

void
shl32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest <<= src;
    wpcsetval64(ret, dest);

    return;
}

void
shl64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest <<= src;
    wpcsetval64(ret, dest);

    return;
}

void
shr32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t sign = dest & 0x80000000;

    sign = (sign) & 0xff << (32 - src) : 0;
    dest >>= cnt;
    dest |= sign;
    wpcsetval64(ret, dest);

    return;
}

void
shr64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);
    int64_t sign = dest & INT64_C(0x8000000000000000)

    sign = (sign) & 0xff << (64 - src) : 0;
    dest >>= cnt;
    dest |= sign;
    wpcsetval64(ret, dest);

    return;
}

void
shrl32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = 0xffffffff >> src;

    sign = (sign) & 0xffffffff << (32 - src) : 0;
    dest >>= cnt;
    dest &= mask;
    wpcsetval64(ret, dest);

    return;
}

void
shrl64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);
    int64_t mask = INT64_C(0xffffffffffffffff) >> src;

    sign = (sign) & 0xff << (64 - src) : 0;
    dest >>= cnt;
    dest &= mask;
    wpcsetval64(ret, dest);

    return;
}

void
ror32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = 0xffffffff >> (32 - src);
    int32_t bits = dest & mask;

    bits <<= 32 - cnt;
    dest >>= cnt;
    dest |= bits;
    wpcsetval64(ret, dest);

    return;
}

void
ror64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);
    int64_t mask = INT64_C(0xffffffffffffffff) >> (64 - src);
    int64_t bits = dest & mask;

    bits <<= 64 - cnt;
    dest >>= cnt;
    dest |= bits;
    wpcsetval64(ret, dest);

    return;
}

void
rol32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = 0xffffffff >> (32 - src);
    int32_t bits = dest & mask;

    bits >>= 32 - cnt;
    dest <<= cnt;
    dest |= bits;
    wpcsetval64(ret, dest);

    return;
}

void
rol64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);
    int64_t mask = INT64_C(0xffffffffffffffff) >> (64 - src);
    int64_t bits = dest & mask;

    bits >>= 64 - cnt;
    dest <<= cnt;
    dest |= bits;
    wpcsetval64(ret, dest);

    return;
}

void
inc32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);

    src++;
    wpcsetval64(ret, dest);

    return;
}

void
inc64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);

    src++;
    wpcsetval64(ret, dest);

    return;
}

void
dec32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);

    src--;
    wpcsetval64(ret, dest);

    return;
}

void
dec64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);

    src--;
    wpcsetval64(ret, dest);

    return;
}

void
add32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest += src;
    wpcsetval64(ret, dest);

    return;
}

void
add64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest += src;
    wpcsetval64(ret, dest);

    return;
}

void
sub32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest -= src;
    wpcsetval64(ret, dest);

    return;
}

void
sub64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest -= src;
    wpcsetval64(ret, dest);

    return;
}

void
mul32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest *= src;
    wpcsetval64(ret, dest);

    return;
}

void
mul64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest *= src;
    wpcsetval64(ret, dest);

    return;
}

void
div32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest /= src;
    wpcsetval64(ret, dest);

    return;
}

void
div64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest /= src;
    wpcsetval64(ret, dest);

    return;
}

void
mod32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest %= src;
    wpcsetval64(ret, dest);

    return;
}

void
mod64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest %= src;
    wpcsetval64(ret, dest);

    return;
}

