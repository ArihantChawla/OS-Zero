#include <stdint.h>
#include <zpc/zpc.h>
#include <zpc/op.h>
#include <zero/trix.h>

int64_t
abs64(int64_t arg1, int64_t dummy1, int64_t dummy2)
{
    int64_t src = arg1;
    int64_t res = zeroabs(src);

    return res;
}

int64_t
min64(int64_t arg1, int64_t arg2, int64_t dummy)
{
    int64_t src = arg1;
    int64_t res = min(src, arg2);

    return res;
}

int64_t
max64(int64_t arg1, int64_t arg2, int64_t dummy)
{
    int64_t src = arg1;
    int64_t res = max(src, arg2);

    return res;
}

int64_t
bitset64(int64_t arg1, int64_t arg2, int64_t dummy)
{
    int64_t src = arg1;
    int64_t mask = INT64_C(1) << arg2;
    int64_t res = src & mask;

    return res;
}

int64_t
setbit64(int64_t arg1, int64_t arg2, int64_t dummy)
{
    int64_t src = arg1;
    int64_t mask = 1U << arg2;
    int64_t res = src | mask;

    return res;
}

int64_t
clrbit64(int64_t arg1, int64_t arg2, int64_t dummy)
{
    int64_t src = arg1;
    int64_t mask = 1U << arg2;
    int64_t res = src & ~mask;

    return res;
}

int64_t
setbits64(int64_t arg1, int64_t arg2, int64_t arg3)
{
    int64_t src = arg1;
    int64_t ofs = arg2;
    int64_t n = arg3;
    int64_t res = setbits(src, ofs, n, 1);

    return res;
}

int64_t
clrbits64(int64_t arg1, int64_t arg2, int64_t arg3)
{
    int64_t src = arg1;
    int64_t ofs = arg2;
    int64_t n = arg3;
    int64_t res = setbits(src, ofs, n, 0);

    return res;
}

int64_t
mergebits64(int64_t arg1, int64_t arg2, int64_t arg3)
{
    int64_t src = arg1;
    int64_t dest = arg2;
    int64_t mask = arg3;
    int64_t res = mergebits(dest, src, mask);

    return res;
}

int64_t
copybits64(int64_t arg1, int64_t arg2, int64_t arg3)
{
    int64_t src = arg1;
    int64_t dest = arg2;
    int64_t mask = arg3;
    int64_t res = copybits(dest, src, mask);

    return res;
}

int64_t
is2pow64(int64_t arg1, int64_t dummy1, int64_t dummy2)
{
    int64_t src = arg1;
    int64_t res = powerof2(src);

    return res;
}

int64_t
rnd2up64(int64_t arg1, int64_t arg2, int64_t dummy)
{
    int64_t src = arg1;
    int64_t pow2 = arg2;
    int64_t res = rounduppow2(src, pow2);

    return res;
}

int64_t
rnd2down64(int64_t arg1, int64_t arg2, int64_t dummy)
{
    int64_t src = arg1;
    int64_t pow2 = arg2;
    int64_t res = rounddownpow2(src, pow2);

    return res;
}

int64_t
trailz64(int64_t arg1, int64_t dummy1, int64_t dummy2)
{
    uint64_t src = arg1;
    uint64_t res;

    tzero64(src, res);

    return res;
}

int64_t
leadz64(int64_t arg1, int64_t dummy1, int64_t dummy2)
{
    uint64_t src = arg1;
    uint64_t res;

    lzero64(src, res);

    return res;
}

int64_t
ceil2pow64(int64_t arg1, int64_t dummy1, int64_t dummy2)
{
    uint64_t src = arg1;
    uint64_t res;

    res = ceilpow2_64(src);

    return res;
}

int64_t
isleapyear64(int64_t arg1, int64_t dummy1, int64_t ydummy2)
{
    uint64_t src = arg1;
    uint64_t res = leapyear(src);

    return res;
}

