#include <stdint.h>
#include <stdio.h>
#include <limits.h>
#include <zpc/zpc.h>
#include <zpc/op.h>

int64_t
not64(struct zpctoken *arg1, struct zpctoken *dummy)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res;

    res = ~src;

    return res;
}

int64_t
and64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res &= src;

    return res;
}

int64_t
or64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res |= src;

    return res;
}

int64_t
xor64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res ^= src;

    return res;
}

int64_t
shl64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t  cnt = arg1->data.ui64.i64;
    uint64_t res = arg2->data.ui64.u64;

    res <<= cnt;

    return res;
}

int64_t
shra64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t  cnt = arg1->data.ui64.i64;
    uint64_t res = arg2->data.ui64.u64;
    uint64_t sign = res & UINT64_C(0x8000000000000000);

    sign = (sign & UINT64_C(0xffffffffffffffff)) << (64 - cnt);
    res >>= cnt;
    res |= sign;

    return res;
}

int64_t
shr64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t  cnt = arg1->data.ui64.i64;
    uint64_t res = arg2->data.ui64.u64;
    uint64_t mask = UINT64_C(0xffffffffffffffff) >> cnt;

    res >>= cnt;
    res &= mask;

    return res;
}

int64_t
ror64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t  cnt = arg1->data.ui64.i64;
    uint64_t res = arg2->data.ui64.u64;
    uint64_t mask = UINT64_C(0xffffffffffffffff) >> (64 - cnt);
    int64_t  bits = res & mask;

    bits <<= 64 - cnt;
    res >>= cnt;
    res |= bits;

    return res;
}

int64_t
rol64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t  cnt = arg1->data.ui64.i64;
    uint64_t res = arg2->data.ui64.u64;
    uint64_t mask = UINT64_C(0xffffffffffffffff) << (64 - cnt);
    uint64_t bits = res & mask;

    bits >>= 64 - cnt;
    res <<= cnt;
    res |= bits;

    return res;
}

int64_t
inc64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;

#if (SMARTTYPES)
    arg1->flags = 0;
    if (src < INT64_MAX) {
        src++;
    } else if (arg1->type == ZPCINT64) {
        if (arg1->type == ZPCUSERSIGNED) {
            arg1->flags |= ZPCOVERFLOW;
            src++;
        } else {
            src++;
            if (!src) {
                if (!src) {
                    arg1->flags |= ZPCZERO;
                }
            } else if (src > INT64_MAX) {
                arg1->type = ZPCUINT64;
                arg1->sign = ZPCUNSIGNED;
            }
        }
    } else if (arg1->type == ZPCUINT64) {
        if (src == UINT64_MAX) {
            arg1->flags |= ZPCOVERFLOW;
        }
        src++;
    }
#else
    src++;
#endif

    return src;
}

int64_t
dec64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;

#if (SMARTTYPES)
    arg1->flags = 0;
    if (src == INT64_MIN) {
        if (arg1->type == ZPCINT64) {
            arg1->flags |= ZPCUNDERFLOW;
        }
    } else if (arg1->type == ZPCUINT64) {
        if (!src) {
            arg1->flags |= ZPCUNDERFLOW;
        }
    }
    src--;
#else
    src--;
#endif

    return src;
}

int64_t
add64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res += src;

    return res;
}

int64_t
sub64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res -= src;

    return res;
}

int64_t
mul64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res *= src;

    return res;
}

int64_t
div64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res /= src;

    return res;
}

int64_t
mod64(struct zpctoken *arg1, struct zpctoken *arg2)
{
    int64_t src = arg1->data.ui64.i64;
    int64_t res = arg2->data.ui64.i64;

    res %= src;

    return res;
}

void
opnot(struct zpcopcode *op)
{
    uint_fast8_t arg1 = op->arg1t;
    
}
