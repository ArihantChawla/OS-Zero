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

