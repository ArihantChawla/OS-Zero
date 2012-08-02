#include <wpc/op.h>

void
copnot8(void *arg1, void *dummy, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest;

    dest = ~src;
    wpcsetval8(ret, dest);

    return;
}

void
copnot16(void *arg1, void *dummy, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest;

    dest = ~src;
    wpcsetval16(ret, dest);

    return;
}

void
copnot32(void *arg1, void *dummy, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest;

    dest = ~src;
    wpcsetval32(ret, dest);

    return;
}

void
copnot64(void *arg1, void *dummy, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest;

    dest = ~src;
    wpcsetval64(ret, dest);

    return;
}

void
copand8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest &= src;
    wpcsetval8(ret, dest);

    return;
}

void
copand16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest &= src;
    wpcsetval16(ret, dest);

    return;
}

void
copand32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest &= src;
    wpcsetval32(ret, dest);

    return;
}

void
copand64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest &= src;
    wpcsetval64(ret, dest);

    return;
}

void
copor8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest |= src;
    wpcsetval8(ret, dest);

    return;
}

void
copor16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest |= src;
    wpcsetval16(ret, dest);

    return;
}

void
copor32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest |= src;
    wpcsetval32(ret, dest);

    return;
}

void
copor64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest |= src;
    wpcsetval64(ret, dest);

    return;
}

void
copxor8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest ^= src;
    wpcsetval8(ret, dest);

    return;
}

void
copxor16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest ^= src;
    wpcsetval16(ret, dest);

    return;
}

void
copxor32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest ^= src;
    wpcsetval32(ret, dest);

    return;
}

void
copxor64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest ^= src;
    wpcsetval64(ret, dest);

    return;
}

void
copshl8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest <<= src;
    wpcsetval8(ret, dest);

    return;
}

void
copshl16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest <<= src;
    wpcsetval16(ret, dest);

    return;
}

void
copshl32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest <<= src;
    wpcsetval32(ret, dest);

    return;
}

void
copshl64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest <<= src;
    wpcsetval64(ret, dest);

    return;
}

void
copshr8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);
    int8_t sign = dest & 0x80;

    sign = (sign) & 0xff << (8 - src) : 0;
    dest >>= cnt;
    dest |= sign;
    wpcsetval8(ret, dest);

    return;
}

copshr16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);
    int16_t sign = dest & 0x8000;

    sign = (sign) & 0xff << (16 - src) : 0;
    dest >>= cnt;
    dest |= sign;
    wpcsetval16(ret, dest);

    return;
}

void
copshr32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t sign = dest & 0x80000000;

    sign = (sign) & 0xff << (32 - src) : 0;
    dest >>= cnt;
    dest |= sign;
    wpcsetval32(ret, dest);

    return;
}

void
copshr64(void *arg1, void *arg2, void *ret)
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
copshrl8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);
    int8_t mask = 0xff >> src;

    sign = (sign) & 0xff << (8 - src) : 0;
    dest >>= cnt;
    dest &= mask;
    wpcsetval8(ret, dest);

    return;
}

void
copshrl16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);
    int16_t mask = 0xffff >> src;

    sign = (sign) & 0xff << (16 - src) : 0;
    dest >>= cnt;
    dest &= mask;
    wpcsetval16(ret, dest);

    return;
}

void
copshrl32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = 0xffffffff >> src;

    sign = (sign) & 0xffffffff << (32 - src) : 0;
    dest >>= cnt;
    dest &= mask;
    wpcsetval32(ret, dest);

    return;
}

void
copshrl64(void *arg1, void *arg2, void *ret)
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
copror8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);
    int8_t mask = 0xff >> (8 - src);
    int8_t bits = dest & mask;

    bits <<= 8 - cnt;
    dest >>= cnt;
    dest |= bits;
    wpcsetval8(ret, dest);

    return;
}

void
copror16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);
    int16_t mask = 0xffff >> (16 - src);
    int16_t bits = dest & mask;

    bits <<= 16 - cnt;
    dest >>= cnt;
    dest |= bits;
    wpcsetval16(ret, dest);

    return;
}

void
copror32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = 0xffffffff >> (32 - src);
    int32_t bits = dest & mask;

    bits <<= 32 - cnt;
    dest >>= cnt;
    dest |= bits;
    wpcsetval32(ret, dest);

    return;
}

void
copror64(void *arg1, void *arg2, void *ret)
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
coprol8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);
    int8_t mask = 0xff >> (8 - src);
    int8_t bits = dest & mask;

    bits >>= 8 - cnt;
    dest <<= cnt;
    dest |= bits;
    wpcsetval8(ret, dest);

    return;
}

void
coprol16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);
    int16_t mask = 0xffff >> (16 - src);
    int16_t bits = dest & mask;

    bits >>= 16 - cnt;
    dest <<= cnt;
    dest |= bits;
    wpcsetval16(ret, dest);

    return;
}

void
coprol32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);
    int32_t mask = 0xffffffff >> (32 - src);
    int32_t bits = dest & mask;

    bits >>= 32 - cnt;
    dest <<= cnt;
    dest |= bits;
    wpcsetval32(ret, dest);

    return;
}

void
coprol64(void *arg1, void *arg2, void *ret)
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
copinc8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);

    src++;
    wpcgetval8(arg1) = dest;

    return;
}

void
copinc16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);

    src++;
    wpcsetval16(ret, dest);

    return;
}

void
copinc32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);

    src++;
    wpcsetval32(ret, dest);

    return;
}

void
copinc64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);

    src++;
    wpcsetval64(ret, dest);

    return;
}

void
copdec8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);

    src--;
    wpcgetval8(arg1) = dest;

    return;
}

void
copdec16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);

    src--;
    wpcsetval16(ret, dest);

    return;
}

void
copdec32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);

    src--;
    wpcsetval32(ret, dest);

    return;
}

void
copdec64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);

    src--;
    wpcsetval64(ret, dest);

    return;
}

void
copadd8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest += src;
    wpcsetval8(ret, dest);

    return;
}

void
copadd16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest += src;
    wpcsetval16(ret, dest);

    return;
}

void
copadd32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest += src;
    wpcsetval32(ret, dest);

    return;
}

void
copadd64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest += src;
    wpcsetval64(ret, dest);

    return;
}

void
copsub8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest -= src;
    wpcsetval8(ret, dest);

    return;
}

void
copsub16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest -= src;
    wpcsetval16(ret, dest);

    return;
}

void
copsub32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest -= src;
    wpcsetval32(ret, dest);

    return;
}

void
copsub64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest -= src;
    wpcsetval64(ret, dest);

    return;
}

void
copmul8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest *= src;
    wpcsetval8(ret, dest);

    return;
}

void
copmul16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest *= src;
    wpcsetval16(ret, dest);

    return;
}

void
copmul32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest *= src;
    wpcsetval32(ret, dest);

    return;
}

void
copmul64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest *= src;
    wpcsetval64(ret, dest);

    return;
}

void
copdiv8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest /= src;
    wpcsetval8(ret, dest);

    return;
}

void
copdiv16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest /= src;
    wpcsetval16(ret, dest);

    return;
}

void
copdiv32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest /= src;
    wpcsetval32(ret, dest);

    return;
}

void
copdiv64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest /= src;
    wpcsetval64(ret, dest);

    return;
}

void
copmod8(void *arg1, void *arg2, void *ret)
{
    int8_t src = wpcgetval8(arg1);
    int8_t dest = wpcgetval8(arg2);

    dest %= src;
    wpcsetval8(ret, dest);

    return;
}

void
copmod16(void *arg1, void *arg2, void *ret)
{
    int16_t src = wpcgetval16(arg1);
    int16_t dest = wpcgetval16(arg2);

    dest %= src;
    wpcsetval16(ret, dest);

    return;
}

void
copmod32(void *arg1, void *arg2, void *ret)
{
    int32_t src = wpcgetval32(arg1);
    int32_t dest = wpcgetval32(arg2);

    dest %= src;
    wpcsetval32(ret, dest);

    return;
}

void
copmod64(void *arg1, void *arg2, void *ret)
{
    int64_t src = wpcgetval64(arg1);
    int64_t dest = wpcgetval64(arg2);

    dest %= src;
    wpcsetval64(ret, dest);

    return;
}

