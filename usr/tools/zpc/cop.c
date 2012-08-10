#include <stdint.h>
#include <stdio.h>
#include <zpc/zpc.h>
#include <zpc/op.h>

void
not64(void *arg1, void *dummy, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest;

    dest = ~src;
    zpcsetval64(ret, dest);

    return;
}

void
and64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest &= src;
    zpcsetval64(ret, dest);

    return;
}

void
or64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest |= src;
    zpcsetval64(ret, dest);

    return;
}

void
xor64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest ^= src;
    zpcsetval64(ret, dest);

    return;
}

void
shl64(void *arg1, void *arg2, void *ret)
{
    int64_t cnt = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest <<= cnt;
    zpcsetval64(ret, dest);

    return;
}

void
shr64(void *arg1, void *arg2, void *ret)
{
    int64_t cnt = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);
    int64_t sign = dest & INT64_C(0x8000000000000000);

    sign = (sign & INT64_C(0xffffffffffffffff)) << (64 - cnt);
    dest >>= cnt;
    dest |= sign;
    zpcsetval64(ret, dest);

    return;
}

void
shrl64(void *arg1, void *arg2, void *ret)
{
    int64_t cnt = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);
    int64_t mask = INT64_C(0xffffffffffffffff) >> cnt;

    dest >>= cnt;
    dest &= mask;
    fprintf(stderr, "SHRL: %lld", dest);
    zpcsetval64(ret, dest);

    return;
}

void
ror64(void *arg1, void *arg2, void *ret)
{
    int64_t cnt = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);
    int64_t mask = INT64_C(0xffffffffffffffff) >> (64 - cnt);
    int64_t bits = dest & mask;

    bits <<= 64 - cnt;
    dest >>= cnt;
    dest |= bits;
    zpcsetval64(ret, dest);

    return;
}

void
rol64(void *arg1, void *arg2, void *ret)
{
    int64_t cnt = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);
    int64_t mask = INT64_C(0xffffffffffffffff) << (64 - cnt);
    int64_t bits = dest & mask;

    bits >>= 64 - cnt;
    dest <<= cnt;
    dest |= bits;
    zpcsetval64(ret, dest);

    return;
}

void
inc64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);

    src++;
    zpcsetval64(ret, src);

    return;
}

void
dec64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);

    src--;
    zpcsetval64(ret, src);

    return;
}

void
add64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest += src;
    zpcsetval64(ret, dest);

    return;
}

void
fadd64(void *arg1, void *arg2, void *ret)
{
    double src = zpcgetvalf64(arg1);
    double dest = zpcgetvalf64(arg2);

    dest += src;
    zpcsetval64(ret, dest);

    return;
}

void
sub64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    fprintf(stderr, "SUB: %lld - %lld\n", dest, src);
    dest -= src;
    zpcsetval64(ret, dest);

    return;
}

void
fsub64(void *arg1, void *arg2, void *ret)
{
    double src = zpcgetvalf64(arg1);
    double dest = zpcgetvalf64(arg2);

    dest -= src;
    zpcsetval64(ret, dest);

    return;
}

void
mul32(void *arg1, void *arg2, void *ret)
{
    int32_t src = zpcgetval32(arg1);
    int32_t dest = zpcgetval32(arg2);

    dest *= src;
    zpcsetval64(ret, dest);

    return;
}

void
mul64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest *= src;
    zpcsetval64(ret, dest);

    return;
}

void
fmul64(void *arg1, void *arg2, void *ret)
{
    double src = zpcgetvalf64(arg1);
    double dest = zpcgetvalf64(arg2);

    dest *= src;
    zpcsetval64(ret, dest);

    return;
}

void
div64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest /= src;
    zpcsetval64(ret, dest);

    return;
}

void
fdiv64(void *arg1, void *arg2, void *ret)
{
    double src = zpcgetvalf64(arg1);
    double dest = zpcgetvalf64(arg2);

    dest /= src;
    zpcsetval64(ret, dest);

    return;
}

void
mod64(void *arg1, void *arg2, void *ret)
{
    int64_t src = zpcgetval64(arg1);
    int64_t dest = zpcgetval64(arg2);

    dest %= src;
    zpcsetval64(ret, dest);

    return;
}

