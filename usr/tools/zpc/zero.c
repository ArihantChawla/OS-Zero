#include <zpc/op.h>
#include <zero/trix.h>

void
abs32(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = zeroabs(src);

    wpcsetval64(ret, dest);
}

void
abs64(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = zeroabs(src);

    wpcsetval64(ret, dest);
}

void
avg32(void *arg1, void *arg2, void *dummy, void *ret)
{
    uint32_t val1 = wpcgetvalu32(arg1);
    uint32_t val2 = wpcgetvalu32(arg2);

    wpcsetval64(ret, uavg(val1, val2));
}

void
avg64(void *arg1, void *arg2, void *dummy, void *ret)
{
    uint64_t val1 = wpcgetvalu64(arg1);
    uint64_t val2 = wpcgetvalu64(arg2);

    wpcsetval64(ret, uavg(val1, val2));
}

void
min32(void *arg1, void *arg2, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = min(src, wpcgetval32(arg2));

    wpcsetval64(ret, dest);
}

void
min64(void *arg1, void *arg2, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = min(src, wpcgetval64(arg2));

    wpcsetval64(ret, dest);
}

void
max32(void *arg1, void *arg2, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = max(src, wpcgetval32(arg2));

    wpcsetval64(ret, dest);
}

void
max64(void *arg1, void *arg2, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = max(src, wpcgetval64(arg2));

    wpcsetval64(ret, dest);
}

void
bitset32(void *arg1, void *arg2, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t mask = 1U << wpcgetval32(arg2);

    wpcsetval64(ret, src & mask);
}

void
bitset64(void *arg1, void *arg2, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t mask = INT64_C(1) << wpcgetval64(arg2);

    wpcsetval64(ret, src & mask);
}

void
setbit32(void *arg1, void *arg2, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t mask = 1U << wpcgetval32(arg2);

    wpcsetval64(ret, src | mask);
}

void
setbit64(void *arg1, void *arg2, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t mask = 1U << wpcgetval64(arg2);

    wpcsetval64(ret, src | mask);
}

void
clrbit32(void *arg1, void *arg2, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t mask = 1U << wpcgetval32(arg2);

    wpcsetval64(ret, src & ~mask);
}

void
clrbit64(void *arg1, void *arg2, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t mask = 1U << wpcgetval64(arg2);

    wpcsetval64(ret, src & ~mask);
}

void
setbits32(void *arg1, void *arg2, void *arg3, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t ofs = wpcgetval32(arg2);
    int32_t n = wpcgetval32(arg3);

    wpcsetval64(ret, setbits(src, ofs, n, 1));
}

void
setbits64(void *arg1, void *arg2, void *arg3, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t ofs = wpcgetval64(arg2);
    int64_t n = wpcgetval64(arg3);

    wpcsetval64(ret, setbits(src, ofs, n, 1));
}

void
clrbits32(void *arg1, void *arg2, void *arg3, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t ofs = wpcgetval32(arg2);
    int32_t n = wpcgetval32(arg3);

    wpcsetval64(ret, setbits(src, ofs, n, 0));
}

void
clrbits64(void *arg1, void *arg2, void *arg3, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t ofs = wpcgetval64(arg2);
    int64_t n = wpcgetval64(arg3);

    wpcsetval64(ret, setbits(src, ofs, n, 0));
}

void
mergebits32(void *arg1, void *arg2, void *arg3, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = wpcgetval32(arg3);

    wpcsetval64(ret, mergebits(dest, src, mask));
}

void
mergebits64(void *arg1, void *arg2, void *arg3, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);
    int64_t mask = wpcgetval64(arg3);

    wpcsetval64(ret, mergebits(dest, src, mask));
}

void
copybits32(void *arg1, void *arg2, void *arg3, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = wpcgetval32(arg3);

    wpcsetval64(ret, copybits(dest, src, mask));
}

void
copybits64(void *arg1, void *arg2, void *arg3, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);
    int64_t mask = wpcgetval64(arg3);

    wpcsetval64(ret, copybits(dest, src, mask));
}

void
is2pow32(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    int32_t src = wpcgetval32(arg1);

    wpcsetval64(ret, powerof2(src));
}

void
is2pow64(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    int64_t src = wpcgetval64(arg1);

    wpcsetval64(ret, powerof2(src));
}

void
rnd2up32(void *arg1, void *arg2, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t pow2 = wpcgetval32(arg2);

    wpcsetval64(ret, roundup2(src, pow2));
}

void
rnd2up64(void *arg1, void *arg2, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t pow2 = wpcgetval64(arg2);

    wpcsetval64(ret, roundup2(src, pow2));
}

void
rnd2down32(void *arg1, void *arg2, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t pow2 = wpcgetval32(arg2);

    wpcsetval64(ret, rounddown2(src, pow2));
}

void
rnd2down64(void *arg1, void *arg2, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t pow2 = wpcgetval64(arg2);

    wpcsetval64(ret, rounddown2(src, pow2));
}

void
trailz32(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint32_t src = wpcgetvalu32(arg1);
    uint32_t res;

    tzero32(src, res);
    wpcsetval64(ret, res);
}

void
trailz64(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint64_t src = wpcgetvalu64(arg1);
    uint64_t res;

    tzero64(src, res);
    wpcsetval64(ret, res);
}

void
leadz32(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint32_t src = wpcgetvalu32(arg1);
    uint32_t res;

    lzero32(src, res);
    wpcsetval64(ret, res);
}

void
leadz64(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint64_t src = wpcgetvalu64(arg1);
    uint64_t res;

    lzero64(src, res);
    wpcsetval64(ret, res);
}

void
ceil2pow32(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint32_t src = wpcgetvalu32(arg1);

    wpcsetval64(ret, ceil2_32(src));
}

void
ceil2pow64(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint64_t src = wpcgetvalu64(arg1);

    wpcsetval64(ret, ceil2_64(src));
}

void
leapyear32(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint32_t src = wpcgetvalu32(arg1);

    wpcsetval64(ret, leapyear(src));
}

void
leapyear64(void *arg1, void *dummy1, void *dummy2, void *ret)
{
    uint64_t src = wpcgetvalu64(arg1);

    wpcsetval64(ret, leapyear(src));
}

