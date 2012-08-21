#include <stdint.h>
#include <zero/trix.h>
#include <zpc/zpc.h>

int64_t vpu64not8(struct zpctoken *arg1, struct zpctoken *dummy);
int64_t vpu64not16(struct zpctoken *arg1, struct zpctoken *dummy);
int64_t vpu64not32(struct zpctoken *arg1, struct zpctoken *dummy);

int64_t
vpu64not8(struct zpctoken *arg1, struct zpctoken *dummy)
{
    uint8_t val1;
    uint8_t val2;
    uint8_t val3;
    uint8_t val4;

    val1 = arg1->data.v64.v8.vec[0];
    val2 = arg1->data.v64.v8.vec[1];
    val3 = arg1->data.v64.v8.vec[2];
    val4 = arg1->data.v64.v8.vec[3];
    arg1->data.v64.v8.vec[0] = !val1;
    arg1->data.v64.v8.vec[1] = !val2;
    arg1->data.v64.v8.vec[2] = !val3;
    arg1->data.v64.v8.vec[3] = !val4;
    val1 = arg1->data.v64.v8.vec[4];
    val2 = arg1->data.v64.v8.vec[5];
    val3 = arg1->data.v64.v8.vec[6];
    val4 = arg1->data.v64.v8.vec[7];
    arg1->data.v64.v8.vec[4] = !val1;
    arg1->data.v64.v8.vec[5] = !val2;
    arg1->data.v64.v8.vec[6] = !val3;
    arg1->data.v64.v8.vec[7] = !val4;

    return arg1->data.ui64.u64;
}

int64_t
vpu64not16(struct zpctoken *arg1, struct zpctoken *dummy)
{
    uint16_t val1;
    uint16_t val2;
    uint16_t val3;
    uint16_t val4;

    val1 = arg1->data.v64.v16.vec[0];
    val2 = arg1->data.v64.v16.vec[1];
    val3 = arg1->data.v64.v16.vec[2];
    val4 = arg1->data.v64.v16.vec[3];
    arg1->data.v64.v16.vec[0] = !val1;
    arg1->data.v64.v16.vec[1] = !val2;
    arg1->data.v64.v16.vec[2] = !val3;
    arg1->data.v64.v16.vec[3] = !val4;

    return arg1->data.ui64.u64;
}

int64_t
vpu64not32(struct zpctoken *arg1, struct zpctoken *dummy)
{
    uint32_t val1;
    uint32_t val2;

    val1 = arg1->data.v64.v32.vec[0];
    val2 = arg1->data.v64.v32.vec[1];
    arg1->data.v64.v32.vec[0] = !val1;
    arg1->data.v64.v32.vec[1] = !val2;

    return arg1->data.ui64.u64;
}

int64_t
vpu64shr8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint8_t cnt = arg1->data.ui64.u64;;
    uint8_t val1 = arg2->data.v64.v8.vec[0];
    uint8_t val2 = arg2->data.v64.v8.vec[1];
    uint8_t val3 = arg2->data.v64.v8.vec[2];
    uint8_t val4 = arg2->data.v64.v8.vec[3];
    uint8_t mask = 0xffU >> cnt;

    val1 >>= cnt;
    val2 >>= cnt;
    val3 >>= cnt;
    val4 >>= cnt;
    arg2->data.v64.v8.vec[0] = val1 & mask;
    arg2->data.v64.v8.vec[1] = val2 & mask;
    arg2->data.v64.v8.vec[2] = val3 & mask;
    arg2->data.v64.v8.vec[3] = val4 & mask;
    val1 = arg2->data.v64.v8.vec[4];
    val2 = arg2->data.v64.v8.vec[5];
    val3 = arg2->data.v64.v8.vec[6];
    val4 = arg2->data.v64.v8.vec[7];
    val1 >>= cnt;
    val2 >>= cnt;
    val3 >>= cnt;
    val4 >>= cnt;
    arg2->data.v64.v8.vec[4] = val1 & mask;
    arg2->data.v64.v8.vec[5] = val2 & mask;
    arg2->data.v64.v8.vec[6] = val3 & mask;
    arg2->data.v64.v8.vec[7] = val4 & mask;

    return arg2->data.ui64.u64;
}

int64_t
vpu64shr16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint16_t cnt = arg1->data.ui64.u64;;
    uint16_t val1 = arg2->data.v64.v16.vec[0];
    uint16_t val2 = arg2->data.v64.v16.vec[1];
    uint16_t val3 = arg2->data.v64.v16.vec[2];
    uint16_t val4 = arg2->data.v64.v16.vec[3];
    uint16_t mask = 0xffffU >> cnt;

    val1 >>= cnt;
    val2 >>= cnt;
    val3 >>= cnt;
    val4 >>= cnt;
    arg2->data.v64.v16.vec[0] = val1 & mask;
    arg2->data.v64.v16.vec[1] = val2 & mask;
    arg2->data.v64.v16.vec[2] = val3 & mask;
    arg2->data.v64.v16.vec[3] = val4 & mask;

    return arg2->data.ui64.u64;
}

int64_t
vpu64shr32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint32_t cnt = arg1->data.ui64.u64;;
    uint32_t val1 = arg2->data.v64.v32.vec[0];
    uint32_t val2 = arg2->data.v64.v32.vec[1];
    uint32_t mask = 0xffffffffU >> cnt;

    val1 >>= cnt;
    val2 >>= cnt;
    arg2->data.v64.v32.vec[0] = val1 & mask;
    arg2->data.v64.v32.vec[1] = val2 & mask;

    return arg2->data.ui64.u64;
}

int64_t
vpu64shl8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint8_t cnt = arg1->data.ui64.u64;;
    uint8_t val1 = arg2->data.v64.v8.vec[0];
    uint8_t val2 = arg2->data.v64.v8.vec[1];
    uint8_t val3 = arg2->data.v64.v8.vec[2];
    uint8_t val4 = arg2->data.v64.v8.vec[3];

    val1 <<= cnt;
    val2 <<= cnt;
    val3 <<= cnt;
    val4 <<= cnt;
    arg2->data.v64.v8.vec[0] = val1;
    arg2->data.v64.v8.vec[1] = val2;
    arg2->data.v64.v8.vec[2] = val3;
    arg2->data.v64.v8.vec[3] = val4;
    val1 = arg2->data.v64.v8.vec[4];
    val2 = arg2->data.v64.v8.vec[5];
    val3 = arg2->data.v64.v8.vec[6];
    val4 = arg2->data.v64.v8.vec[7];
    val1 <<= cnt;
    val2 <<= cnt;
    val3 <<= cnt;
    val4 <<= cnt;
    arg2->data.v64.v8.vec[4] = val1;
    arg2->data.v64.v8.vec[5] = val2;
    arg2->data.v64.v8.vec[6] = val3;
    arg2->data.v64.v8.vec[7] = val4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64shl16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint16_t cnt = arg1->data.ui64.u64;;
    uint16_t val1 = arg2->data.v64.v16.vec[0];
    uint16_t val2 = arg2->data.v64.v16.vec[1];
    uint16_t val3 = arg2->data.v64.v16.vec[2];
    uint16_t val4 = arg2->data.v64.v16.vec[3];

    val1 <<= cnt;
    val2 <<= cnt;
    val3 <<= cnt;
    val4 <<= cnt;
    arg2->data.v64.v16.vec[0] = val1;
    arg2->data.v64.v16.vec[1] = val2;
    arg2->data.v64.v16.vec[2] = val3;
    arg2->data.v64.v16.vec[3] = val4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64shl32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint32_t cnt = arg1->data.ui64.u64;;
    uint32_t val1 = arg2->data.v64.v32.vec[0];
    uint32_t val2 = arg2->data.v64.v32.vec[1];

    val1 <<= cnt;
    val2 <<= cnt;
    arg2->data.v64.v32.vec[0] = val1;
    arg2->data.v64.v32.vec[1] = val2;

    return arg2->data.ui64.u64;
}

int64_t
vpu64xor8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint8_t src1 = arg1->data.v64.v8.vec[0];
    uint8_t src2 = arg1->data.v64.v8.vec[1];
    uint8_t src3 = arg1->data.v64.v8.vec[2];
    uint8_t src4 = arg1->data.v64.v8.vec[3];
    uint8_t dest1 = arg2->data.v64.v8.vec[0];
    uint8_t dest2 = arg2->data.v64.v8.vec[1];
    uint8_t dest3 = arg2->data.v64.v8.vec[2];
    uint8_t dest4 = arg2->data.v64.v8.vec[3];

    dest1 ^= src1;
    dest2 ^= src2;
    dest3 ^= src3;
    dest4 ^= src4;
    arg2->data.v64.v8.vec[0] = dest1;
    arg2->data.v64.v8.vec[1] = dest2;
    arg2->data.v64.v8.vec[2] = dest3;
    arg2->data.v64.v8.vec[3] = dest4;
    src1 = arg1->data.v64.v8.vec[4];
    src2 = arg1->data.v64.v8.vec[5];
    src3 = arg1->data.v64.v8.vec[6];
    src4 = arg1->data.v64.v8.vec[7];
    dest1 = arg2->data.v64.v8.vec[4];
    dest2 = arg2->data.v64.v8.vec[5];
    dest3 = arg2->data.v64.v8.vec[6];
    dest4 = arg2->data.v64.v8.vec[7];
    dest1 ^= src1;
    dest2 ^= src2;
    dest3 ^= src3;
    dest4 ^= src4;
    arg2->data.v64.v8.vec[4] = dest1;
    arg2->data.v64.v8.vec[5] = dest2;
    arg2->data.v64.v8.vec[6] = dest3;
    arg2->data.v64.v8.vec[7] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64xor16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint16_t src1 = arg1->data.v64.v16.vec[0];
    uint16_t src2 = arg1->data.v64.v16.vec[1];
    uint16_t src3 = arg1->data.v64.v16.vec[2];
    uint16_t src4 = arg1->data.v64.v16.vec[3];
    uint16_t dest1 = arg2->data.v64.v16.vec[0];
    uint16_t dest2 = arg2->data.v64.v16.vec[1];
    uint16_t dest3 = arg2->data.v64.v16.vec[2];
    uint16_t dest4 = arg2->data.v64.v16.vec[3];

    dest1 ^= src1;
    dest2 ^= src2;
    dest3 ^= src3;
    dest4 ^= src4;
    arg2->data.v64.v16.vec[0] = dest1;
    arg2->data.v64.v16.vec[1] = dest2;
    arg2->data.v64.v16.vec[2] = dest3;
    arg2->data.v64.v16.vec[3] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64xor32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint32_t src1 = arg1->data.v64.v32.vec[0];
    uint32_t src2 = arg1->data.v64.v32.vec[1];
    uint32_t dest1 = arg2->data.v64.v32.vec[0];
    uint32_t dest2 = arg2->data.v64.v32.vec[1];

    dest1 ^= src1;
    dest2 ^= src2;
    arg2->data.v64.v32.vec[0] = dest1;
    arg2->data.v64.v32.vec[1] = dest2;

    return arg2->data.ui64.u64;
}

int64_t
vpu64or8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint8_t src1 = arg1->data.v64.v8.vec[0];
    uint8_t src2 = arg1->data.v64.v8.vec[1];
    uint8_t src3 = arg1->data.v64.v8.vec[2];
    uint8_t src4 = arg1->data.v64.v8.vec[3];
    uint8_t dest1 = arg2->data.v64.v8.vec[0];
    uint8_t dest2 = arg2->data.v64.v8.vec[1];
    uint8_t dest3 = arg2->data.v64.v8.vec[2];
    uint8_t dest4 = arg2->data.v64.v8.vec[3];

    dest1 |= src1;
    dest2 |= src2;
    dest3 |= src3;
    dest4 |= src4;
    arg2->data.v64.v8.vec[0] = dest1;
    arg2->data.v64.v8.vec[1] = dest2;
    arg2->data.v64.v8.vec[2] = dest3;
    arg2->data.v64.v8.vec[3] = dest4;
    src1 = arg1->data.v64.v8.vec[4];
    src2 = arg1->data.v64.v8.vec[5];
    src3 = arg1->data.v64.v8.vec[6];
    src4 = arg1->data.v64.v8.vec[7];
    dest1 = arg2->data.v64.v8.vec[4];
    dest2 = arg2->data.v64.v8.vec[5];
    dest3 = arg2->data.v64.v8.vec[6];
    dest4 = arg2->data.v64.v8.vec[7];
    dest1 |= src1;
    dest2 |= src2;
    dest3 |= src3;
    dest4 |= src4;
    arg2->data.v64.v8.vec[4] = dest1;
    arg2->data.v64.v8.vec[5] = dest2;
    arg2->data.v64.v8.vec[6] = dest3;
    arg2->data.v64.v8.vec[7] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64or16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint16_t src1 = arg1->data.v64.v16.vec[0];
    uint16_t src2 = arg1->data.v64.v16.vec[1];
    uint16_t src3 = arg1->data.v64.v16.vec[2];
    uint16_t src4 = arg1->data.v64.v16.vec[3];
    uint16_t dest1 = arg2->data.v64.v16.vec[0];
    uint16_t dest2 = arg2->data.v64.v16.vec[1];
    uint16_t dest3 = arg2->data.v64.v16.vec[2];
    uint16_t dest4 = arg2->data.v64.v16.vec[3];

    dest1 |= src1;
    dest2 |= src2;
    dest3 |= src3;
    dest4 |= src4;
    arg2->data.v64.v16.vec[0] = dest1;
    arg2->data.v64.v16.vec[1] = dest2;
    arg2->data.v64.v16.vec[2] = dest3;
    arg2->data.v64.v16.vec[3] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64or32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint32_t src1 = arg1->data.v64.v32.vec[0];
    uint32_t src2 = arg1->data.v64.v32.vec[1];
    uint32_t dest1 = arg2->data.v64.v32.vec[0];
    uint32_t dest2 = arg2->data.v64.v32.vec[1];

    dest1 |= src1;
    dest2 |= src2;
    arg2->data.v64.v32.vec[0] = dest1;
    arg2->data.v64.v32.vec[1] = dest2;

    return arg2->data.ui64.u64;
}

int64_t
vpu64and8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint8_t src1 = arg1->data.v64.v8.vec[0];
    uint8_t src2 = arg1->data.v64.v8.vec[1];
    uint8_t src3 = arg1->data.v64.v8.vec[2];
    uint8_t src4 = arg1->data.v64.v8.vec[3];
    uint8_t dest1 = arg2->data.v64.v8.vec[0];
    uint8_t dest2 = arg2->data.v64.v8.vec[1];
    uint8_t dest3 = arg2->data.v64.v8.vec[2];
    uint8_t dest4 = arg2->data.v64.v8.vec[3];

    dest1 &= src1;
    dest2 &= src2;
    dest3 &= src3;
    dest4 &= src4;
    arg2->data.v64.v8.vec[0] = dest1;
    arg2->data.v64.v8.vec[1] = dest2;
    arg2->data.v64.v8.vec[2] = dest3;
    arg2->data.v64.v8.vec[3] = dest4;
    src1 = arg1->data.v64.v8.vec[4];
    src2 = arg1->data.v64.v8.vec[5];
    src3 = arg1->data.v64.v8.vec[6];
    src4 = arg1->data.v64.v8.vec[7];
    dest1 = arg2->data.v64.v8.vec[4];
    dest2 = arg2->data.v64.v8.vec[5];
    dest3 = arg2->data.v64.v8.vec[6];
    dest4 = arg2->data.v64.v8.vec[7];
    dest1 &= src1;
    dest2 &= src2;
    dest3 &= src3;
    dest4 &= src4;
    arg2->data.v64.v8.vec[4] = dest1;
    arg2->data.v64.v8.vec[5] = dest2;
    arg2->data.v64.v8.vec[6] = dest3;
    arg2->data.v64.v8.vec[7] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64and16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint16_t src1 = arg1->data.v64.v16.vec[0];
    uint16_t src2 = arg1->data.v64.v16.vec[1];
    uint16_t src3 = arg1->data.v64.v16.vec[2];
    uint16_t src4 = arg1->data.v64.v16.vec[3];
    uint16_t dest1 = arg2->data.v64.v16.vec[0];
    uint16_t dest2 = arg2->data.v64.v16.vec[1];
    uint16_t dest3 = arg2->data.v64.v16.vec[2];
    uint16_t dest4 = arg2->data.v64.v16.vec[3];

    dest1 &= src1;
    dest2 &= src2;
    dest3 &= src3;
    dest4 &= src4;
    arg2->data.v64.v16.vec[0] = dest1;
    arg2->data.v64.v16.vec[1] = dest2;
    arg2->data.v64.v16.vec[2] = dest3;
    arg2->data.v64.v16.vec[3] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64and32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    uint32_t src1 = arg1->data.v64.v32.vec[0];
    uint32_t src2 = arg1->data.v64.v32.vec[1];
    uint32_t dest1 = arg2->data.v64.v32.vec[0];
    uint32_t dest2 = arg2->data.v64.v32.vec[1];

    dest1 &= src1;
    dest2 &= src2;
    arg2->data.v64.v32.vec[0] = dest1;
    arg2->data.v64.v32.vec[1] = dest2;

    return arg2->data.ui64.u64;
}

int64_t
vpu64add8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int8_t src1 = arg1->data.v64.v8.vec[0];
    int8_t src2 = arg1->data.v64.v8.vec[1];
    int8_t src3 = arg1->data.v64.v8.vec[2];
    int8_t src4 = arg1->data.v64.v8.vec[3];
    int8_t dest1 = arg2->data.v64.v8.vec[0];
    int8_t dest2 = arg2->data.v64.v8.vec[1];
    int8_t dest3 = arg2->data.v64.v8.vec[2];
    int8_t dest4 = arg2->data.v64.v8.vec[3];

    dest1 += src1;
    dest2 += src2;
    dest3 += src3;
    dest4 += src4;
    arg2->data.v64.v8.vec[0] = dest1;
    arg2->data.v64.v8.vec[1] = dest2;
    arg2->data.v64.v8.vec[2] = dest3;
    arg2->data.v64.v8.vec[3] = dest4;
    src1 = arg1->data.v64.v8.vec[4];
    src2 = arg1->data.v64.v8.vec[5];
    src3 = arg1->data.v64.v8.vec[6];
    src4 = arg1->data.v64.v8.vec[7];
    dest1 = arg2->data.v64.v8.vec[4];
    dest2 = arg2->data.v64.v8.vec[5];
    dest3 = arg2->data.v64.v8.vec[6];
    dest4 = arg2->data.v64.v8.vec[7];
    dest1 += src1;
    dest2 += src2;
    dest3 += src3;
    dest4 += src4;
    arg2->data.v64.v8.vec[4] = dest1;
    arg2->data.v64.v8.vec[5] = dest2;
    arg2->data.v64.v8.vec[6] = dest3;
    arg2->data.v64.v8.vec[7] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64add16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int16_t src1 = arg1->data.v64.v16.vec[0];
    int16_t src2 = arg1->data.v64.v16.vec[1];
    int16_t src3 = arg1->data.v64.v16.vec[2];
    int16_t src4 = arg1->data.v64.v16.vec[3];
    int16_t dest1 = arg2->data.v64.v16.vec[0];
    int16_t dest2 = arg2->data.v64.v16.vec[1];
    int16_t dest3 = arg2->data.v64.v16.vec[2];
    int16_t dest4 = arg2->data.v64.v16.vec[3];

    dest1 += src1;
    dest2 += src2;
    dest3 += src3;
    dest4 += src4;
    arg2->data.v64.v16.vec[0] = dest1;
    arg2->data.v64.v16.vec[1] = dest2;
    arg2->data.v64.v16.vec[2] = dest3;
    arg2->data.v64.v16.vec[3] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64add32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int32_t src1 = arg1->data.v64.v32.vec[0];
    int32_t src2 = arg1->data.v64.v32.vec[1];
    int32_t dest1 = arg2->data.v64.v32.vec[0];
    int32_t dest2 = arg2->data.v64.v32.vec[1];

    dest1 += src1;
    dest2 += src2;
    arg2->data.v64.v32.vec[0] = dest1;
    arg2->data.v64.v32.vec[1] = dest2;

    return arg2->data.ui64.u64;
}

int64_t
vpu64addus8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int16_t src1 = arg1->data.v64.v8.vec[0];
    int16_t src2 = arg1->data.v64.v8.vec[1];
    int16_t src3 = arg1->data.v64.v8.vec[2];
    int16_t src4 = arg1->data.v64.v8.vec[3];
    int16_t dest1 = arg2->data.v64.v8.vec[0];
    int16_t dest2 = arg2->data.v64.v8.vec[1];
    int16_t dest3 = arg2->data.v64.v8.vec[2];
    int16_t dest4 = arg2->data.v64.v8.vec[3];

    dest1 += src1;
    dest2 += src2;
    dest3 += src3;
    dest4 += src4;
    arg2->data.v64.v8.vec[0] = satu8(dest1);
    arg2->data.v64.v8.vec[1] = satu8(dest2);
    arg2->data.v64.v8.vec[2] = satu8(dest3);
    arg2->data.v64.v8.vec[3] = dest4;
    src1 = arg1->data.v64.v8.vec[4];
    src2 = arg1->data.v64.v8.vec[5];
    src3 = arg1->data.v64.v8.vec[6];
    src4 = arg1->data.v64.v8.vec[7];
    dest1 = arg2->data.v64.v8.vec[4];
    dest2 = arg2->data.v64.v8.vec[5];
    dest3 = arg2->data.v64.v8.vec[6];
    dest4 = arg2->data.v64.v8.vec[7];
    dest1 += src1;
    dest2 += src2;
    dest3 += src3;
    dest4 += src4;
    arg2->data.v64.v8.vec[4] = dest1;
    arg2->data.v64.v8.vec[5] = dest2;
    arg2->data.v64.v8.vec[6] = dest3;
    arg2->data.v64.v8.vec[7] = dest4;

    return arg2->data.ui64.u64;
}

int64_t
vpu64addus16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int16_t src1 = arg1->data.v64.v16.vec[0];
    int16_t src2 = arg1->data.v64.v16.vec[1];
    int16_t src3 = arg1->data.v64.v16.vec[2];
    int16_t src4 = arg1->data.v64.v16.vec[3];
    int16_t dest1 = arg2->data.v64.v16.vec[0];
    int16_t dest2 = arg2->data.v64.v16.vec[1];
    int16_t dest3 = arg2->data.v64.v16.vec[2];
    int16_t dest4 = arg2->data.v64.v16.vec[3];

    dest1 += src1;
    dest2 += src2;
    dest3 += src3;
    dest4 += src4;
    arg2->data.v64.v16.vec[0] = satu16(dest1);
    arg2->data.v64.v16.vec[1] = satu16(dest2);
    arg2->data.v64.v16.vec[2] = satu16(dest3);
    arg2->data.v64.v16.vec[3] = satu16(dest4);

    return arg2->data.ui64.u64;
}

int64_t
vpu64addus32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src1 = arg1->data.v64.v32.vec[0];
    int64_t src2 = arg1->data.v64.v32.vec[1];
    int64_t dest1 = arg2->data.v64.v32.vec[0];
    int64_t dest2 = arg2->data.v64.v32.vec[1];

    dest1 += src1;
    dest2 += src2;
    arg2->data.v64.v32.vec[0] = satu32(dest1);
    arg2->data.v64.v32.vec[1] = satu32(dest2);

    return arg2->data.ui64.u64;
}

