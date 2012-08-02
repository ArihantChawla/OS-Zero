#include <wpc/op.h>

void abs8(void *arg1, void *dummy, void *ret)
{
    int8_t src = wpcgetarg8(arg1);
    int8_t dest = zeroabs(src);

    wpcsetval8(ret, dest);
}

void abs16(void *arg1, void *dummy, void *ret)
{
    int16_t src = wpcgetarg16(arg1);
    int16_t dest = zeroabs(src);

    wpcsetval16(ret, dest);
}

void abs32(void *arg1, void *dummy, void *ret)
{
    int32_t src = wpcgetarg32(arg1);
    int32_t dest = zeroabs(src);

    wpcsetval32(ret, dest);
}

void abs64(void *arg1, void *dummy, void *ret)
{
    int64_t src = wpcgetarg64(arg1);
    int64_t dest = zeroabs(src);

    wpcsetval64(ret, dest);
}

