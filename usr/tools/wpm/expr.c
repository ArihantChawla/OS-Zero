#include <stdint.h>

#if (ASMPREPROC)
int32_t
exprnot(int32_t val1, int32_t val2)
{
    return ~val1;
}

int32_t
exprand(int32_t val1, int32_t val2)
{
    return val1 & val2;
}

int32_t expror(int32_t val1, int32_t val2)
{
    return val1 | val2;
}

int32_t
exprxor(int32_t val1, int32_t val2)
{
    return val1 ^ val2;
}

int32_t
exprshl(int32_t val1, int32_t val2)
{
    return val1 << val2;
}

int32_t
exprshr(int32_t val1, int32_t val2)
{
    return val1 >> val2;
}

int32_t
expradd(int32_t val1, int32_t val2)
{
    return val1 + val2;
}

int32_t
exprsub(int32_t val1, int32_t val2)
{
    return val1 - val2;
}

int32_t
exprmul(int32_t val1, int32_t val2)
{
    return val1 * val2;
}

int32_t
exprdiv(int32_t val1, int32_t val2)
{
    return val1 / val2;
}

int32_t
exprmod(int32_t val1, int32_t val2)
{
    return val1 % val2;
}
#endif

