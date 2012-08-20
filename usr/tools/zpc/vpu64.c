#include <stdint.h>
#include <zpc/zpc.h>

int64_t vpu64not8(struct zpctoken *arg1, struct zpctoken *dummy);
int64_t vpu64not16(struct zpctoken *arg1, struct zpctoken *dummy);
int64_t vpu64not32(struct zpctoken *arg1, struct zpctoken *dummy);

int64_t
vpu64not8(struct zpctoken *arg1, struct zpctoken *dummy)
{
    int8_t val1;
    int8_t val2;
    int8_t val3;
    int8_t val4;

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

    return arg1->data.ui64.i64;
}

int64_t
vpu64not16(struct zpctoken *arg1, struct zpctoken *dummy)
{
    int16_t val1;
    int16_t val2;
    int16_t val3;
    int16_t val4;

    val1 = arg1->data.v64.v16.vec[0];
    val2 = arg1->data.v64.v16.vec[1];
    val3 = arg1->data.v64.v16.vec[2];
    val4 = arg1->data.v64.v16.vec[3];
    arg1->data.v64.v16.vec[0] = !val1;
    arg1->data.v64.v16.vec[1] = !val2;
    arg1->data.v64.v16.vec[2] = !val3;
    arg1->data.v64.v16.vec[3] = !val4;

    return arg1->data.ui64.i64;
}

int64_t
vpu64not32(struct zpctoken *arg1, struct zpctoken *dummy)
{
    int32_t val1;
    int32_t val2;

    val1 = arg1->data.v64.v32.vec[0];
    val2 = arg1->data.v64.v32.vec[1];
    arg1->data.v64.v32.vec[0] = !val1;
    arg1->data.v64.v32.vec[1] = !val2;

    return arg1->data.ui64.i64;
}

int64_t
vpu64shr8(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int8_t cnt = arg1->data.ui64.i64;;
    int8_t res1 = arg2->data.v64.v8.vec[0];
    int8_t res2 = arg2->data.v64.v8.vec[1];
    int8_t res3 = arg2->data.v64.v8.vec[2];
    int8_t res4 = arg2->data.v64.v8.vec[3];
    int8_t mask = 0xffU >> cnt;

    res1 >>= cnt;
    res2 >>= cnt;
    res3 >>= cnt;
    res4 >>= cnt;
    arg2->data.v64.v8.vec[0] = res1 & mask;
    arg2->data.v64.v8.vec[1] = res2 & mask;
    arg2->data.v64.v8.vec[2] = res3 & mask;
    arg2->data.v64.v8.vec[3] = res4 & mask;
    res1 = arg2->data.v64.v8.vec[4];
    res2 = arg2->data.v64.v8.vec[5];
    res3 = arg2->data.v64.v8.vec[6];
    res4 = arg2->data.v64.v8.vec[7];
    res1 >>= cnt;
    res2 >>= cnt;
    res3 >>= cnt;
    res4 >>= cnt;
    arg2->data.v64.v8.vec[4] = res1 & mask;
    arg2->data.v64.v8.vec[5] = res2 & mask;
    arg2->data.v64.v8.vec[6] = res3 & mask;
    arg2->data.v64.v8.vec[7] = res4 & mask;

    return arg2->data.ui64.i64;
}

int64_t
vpu64shr16(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int16_t cnt = arg1->data.ui64.i64;;
    int16_t res1 = arg2->data.v64.v16.vec[0];
    int16_t res2 = arg2->data.v64.v16.vec[1];
    int16_t res3 = arg2->data.v64.v16.vec[2];
    int16_t res4 = arg2->data.v64.v16.vec[3];
    int16_t mask = 0xffffU >> cnt;

    res1 >>= cnt;
    res2 >>= cnt;
    res3 >>= cnt;
    res4 >>= cnt;
    arg2->data.v64.v16.vec[0] = res1 & mask;
    arg2->data.v64.v16.vec[1] = res2 & mask;
    arg2->data.v64.v16.vec[2] = res3 & mask;
    arg2->data.v64.v16.vec[3] = res4 & mask;

    return arg2->data.ui64.i64;
}

int64_t
vpu64shr32(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int32_t cnt = arg1->data.ui64.i64;;
    int32_t res1 = arg2->data.v64.v32.vec[0];
    int32_t res2 = arg2->data.v64.v32.vec[1];
    int32_t mask = 0xffffffffU >> cnt;

    res1 >>= cnt;
    res2 >>= cnt;
    arg2->data.v64.v32.vec[0] = res1 & mask;
    arg2->data.v64.v32.vec[1] = res2 & mask;

    return arg2->data.ui64.i64;
}

