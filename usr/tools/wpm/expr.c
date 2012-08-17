#if (ASMPREPROC)

#include <stdint.h>
#include <wpm/asm.h>

asmword_t
exprnot(asmword_t val1, asmword_t val2)
{
    return ~val1;
}

asmword_t
exprand(asmword_t val1, asmword_t val2)
{
    return val1 & val2;
}

asmword_t expror(asmword_t val1, asmword_t val2)
{
    return val1 | val2;
}

asmword_t
exprxor(asmword_t val1, asmword_t val2)
{
    return val1 ^ val2;
}

asmword_t
exprshl(asmword_t val1, asmword_t val2)
{
    return val1 << val2;
}

asmword_t
exprshr(asmword_t val1, asmword_t val2)
{
    return val1 >> val2;
}

asmword_t
expradd(asmword_t val1, asmword_t val2)
{
    return val1 + val2;
}

asmword_t
exprsub(asmword_t val1, asmword_t val2)
{
    return val1 - val2;
}

asmword_t
exprmul(asmword_t val1, asmword_t val2)
{
    return val1 * val2;
}

asmword_t
exprdiv(asmword_t val1, asmword_t val2)
{
    return val1 / val2;
}

asmword_t
exprmod(asmword_t val1, asmword_t val2)
{
    return val1 % val2;
}
#endif

