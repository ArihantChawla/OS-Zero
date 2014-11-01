#include <limits.h>
#include <stdint.h>
#include <zero/trix.h>
#include <zvm/zvm.h>
#include <zvm/op.h>

extern struct zvm zvm;

void
zvmopnot(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     dest = zvmgetarg(op, arg1t, dptr);

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest = ~dest;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopand(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest &= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopor(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest |= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopxor(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest ^= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopshr(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    zasword_t     sign = ~((zasword_t)0) >> src;

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest >>= src;
    dest &= sign;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopsar(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    zasword_t     sign = (zasword_t)dest >> (CHAR_BIT * sizeof(zasword_t) - 1);

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    sign = -sign << (CHAR_BIT * sizeof(zasword_t) - src);
    dest >>= src;
    dest |= sign;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopshl(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest <<= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopror(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    zasword_t     mask = ~((zasword_t)0) >> (CHAR_BIT * sizeof(zasword_t) - src);
    zasword_t     bits = dest & mask;
    zasword_t     cf = dest & ((zasword_t)1 << (src - 1));

    bits <<= CHAR_BIT * sizeof(zasword_t) - src;
    dest >>= src;
    dest |= bits;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (cf) {
        zvm.msw |= ZVMCF;
    }
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmoprol(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    zasword_t     mask = ~((zasword_t)0) << (CHAR_BIT * sizeof(zasword_t) - src);
    zasword_t     bits = dest & mask;
    zasword_t     cf = dest & ((zasword_t)1 << (src - 1));

    bits >>= CHAR_BIT * sizeof(zasword_t) - src;
    dest <<= src;
    dest |= bits;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (cf) {
        zvm.msw |= ZVMCF;
    }
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopinc(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     dest = zvmgetarg(op, arg1t, dptr);

    dest++;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopdec(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     dest = zvmgetarg(op, arg1t, dptr);

    dest--;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopadd(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    int64_t       res = src + dest;

    dest += src;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (res & INT64_C(0xffffffff00000000)) {
        zvm.msw |= ZVMOF;
    }
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopsub(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    int64_t       res = dest - src;

    dest -= src;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopcmp(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    int64_t       res = dest - src;

    dest -= src;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.msw |= ZVMOF;
    }
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopmul(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);
    int64_t       res = dest * src;

    dest *= src;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (res & INT64_C(0xffffffff00000000)) {
        zvm.msw |= ZVMOF;
    }
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopdiv(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);

    dest /= src;
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.msw |= ZVMOF;
    }
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopmod(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);
    zasword_t     dest = zvmgetarg2(op, arg1t, arg2t, dptr);

    if (powerof2(src)) {
        dest &= src - 1;
    } else {
        dest %= src;
    }
    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.msw |= ZVMOF;
    }
    *dptr = dest;
    zvm.pc += op->size << 2;

    return;
}

void
zvmopjmp(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    zvm.pc = dest;

    return;
}

void
zvmopbz(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.msw & ZVMZF) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopbnz(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.msw & ZVMZF)) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopblt(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.msw & ZVMOF)) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopble(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.msw & ZVMOF) || !(zvm.msw & ZVMZF)) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopbgt(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.msw & ZVMOF) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopbge(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if ((zvm.msw & ZVMOF) || (zvm.msw & ZVMZF)) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopbo(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.msw & ZVMOF) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopbno(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.msw & ZVMOF)) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopbc(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.msw & ZVMCF) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmopbnc(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.msw & ZVMCF)) {
        zvm.pc = dest;
    } else {
        zvm.pc += op->size << 2;
    }
}

void
zvmoppop(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg(op, arg1t, dptr);
    zasword_t     dest = *(zasword_t *)&zvm.physmem[zvm.sp];

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvm.sp -= sizeof(zasword_t);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.pc += op->size << 2;
}

void
zvmoppush(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     src = zvmgetarg1(op, arg1t);

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(src);
    *(zasword_t *)&zvm.physmem[zvm.sp] = src;
    zvm.sp -= sizeof(zasword_t);
    zvm.pc += op->size << 2;
}

void
zvmoppusha(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t     src;
    long          l;

    zvm.msw &= ~(ZVMZF | ZVMOF | ZVMCF);
    for ( l = 0 ; l < ZASNREG ; l++ ) {
        src = zvm.regs[l];
        *(zasword_t *)&zvm.physmem[zvm.sp] = src;
        zvm.sp -= sizeof(zasword_t);
    }
    zvm.pc += op->size << 2;
}

void
zvmopmovl(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint32_t     *dptr;
    uint32_t      src = zvmgetarg1mov(op, arg1t, arg2t);
    uint32_t      dest = zvmgetarg2mov(op, arg1t, arg2t, dptr, uint32_t);

    *dptr = src;
    zvm.pc += op->size << 2;
}

void
zvmopmovb(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint8_t      *dptr;
    uint8_t       src = zvmgetarg1mov(op, arg1t, arg2t);
    uint8_t       dest = zvmgetarg2mov(op, arg1t, arg2t, dptr, uint8_t);

    *dptr = src;
    zvm.pc += op->size << 2;
}

void
zvmopmovw(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint16_t     *dptr;
    uint16_t      src = zvmgetarg1mov(op, arg1t, arg2t);
    uint16_t      dest = zvmgetarg2mov(op, arg1t, arg2t, dptr, uint16_t);

    *dptr = src;
    zvm.pc += op->size << 2;
}

#if (!ZAS32BIT)
void
zvmopmovq(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint64_t     *dptr;
    uint64_t      src = zvmgetarg1mov(op, arg1t, arg2t);
    uint64_t      dest = zvmgetarg2mov(op, arg1t, arg2t, dptr, uint64_t);

    *dptr = src;
    zvm.pc += op->size << 2;
}
#endif

