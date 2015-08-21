/* zero virtual machine instruction set implementation */

#include <zvm/conf.h>

#include <limits.h>
#include <stdint.h>
#include <zero/trix.h>
#include <zvm/zvm.h>
#include <zvm/op.h>
#include <zvm/mem.h>

/* TODO: there be bugs here; proof-read the whole file and fix it... :) */

void
zvmopnot(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     dest = zvmgetarg(op, arg1t, dptr);

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest = ~dest;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest &= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest |= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest ^= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest >>= src;
    dest &= sign;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    sign = -sign << (CHAR_BIT * sizeof(zasword_t) - src);
    dest >>= src;
    dest |= sign;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest <<= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (cf) {
        zvm.cregs[ZVMMSWCREG] |= ZVMCF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (cf) {
        zvm.cregs[ZVMMSWCREG] |= ZVMCF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

    return;
}

void
zvmopinc(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     dest = zvmgetarg(op, arg1t, dptr);

    dest++;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (!dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

    return;
}

void
zvmopdec(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;
    zasword_t     dest = zvmgetarg(op, arg1t, dptr);

    dest--;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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
    zasword_t     res = src + dest;

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(res);
    if (res < dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = res;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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

    dest -= src;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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
//    int64_t       res = dest - src;

    dest -= src;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
#if (ZVM32BIT)
    if (res & INT64_C(0xffffffff00000000)) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
#else
#error fix integer overflow detection in zvmopmul
#endif
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

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
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

    return;
}

void
zvmopjmp(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    zvm.cregs[ZVMPCCREG] = dest;

    return;
}

void
zvmopbz(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.cregs[ZVMMSWCREG] & ZVMZF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopbnz(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMZF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopblt(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMOF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopble(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMOF) || !(zvm.cregs[ZVMMSWCREG] & ZVMZF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopbgt(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.cregs[ZVMMSWCREG] & ZVMOF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopbge(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if ((zvm.cregs[ZVMMSWCREG] & ZVMOF) || (zvm.cregs[ZVMMSWCREG] & ZVMZF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopbo(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.cregs[ZVMMSWCREG] & ZVMOF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopbno(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMOF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopbc(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (zvm.cregs[ZVMMSWCREG] & ZVMCF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmopbnc(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t    dest = zvmgetarg1(op, arg1t);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMCF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += op->size << 2;
    }
}

void
zvmoppop(struct zvmopcode *op)
{
//    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]];
//    zasword_t     src = zvmgetarg(op, arg1t, dptr);
    zasword_t     dest = *(zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]];

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvm.cregs[ZVMSPCREG] -= sizeof(zasword_t);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}

void
zvmoppush(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]];
    zasword_t     src = zvmgetarg1(op, arg1t);

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(src);
    *dptr = src;
    zvm.cregs[ZVMSPCREG] -= sizeof(zasword_t);
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}

void
zvmoppusha(struct zvmopcode *op)
{
    zasword_t  src;
    zasword_t *dptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]];
    long       l;

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    for (l = 0 ; l < ZVMNREG ; l++) {
        src = zvm.regs[l];
        *dptr = src;
        dptr++;
    }
    zvm.cregs[ZVMSPCREG] -= ZVMNREG * sizeof(zasword_t);
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}

void
zvmopmovl(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint32_t     *dptr;
    uint32_t      src = zvmgetarg1mov(op, arg1t, arg2t);

    zvmgetarg2mov(op, arg1t, arg2t, dptr, uint32_t);
    *dptr = src;
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}

void
zvmopmovb(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint8_t      *dptr;
    uint8_t       src = zvmgetarg1mov(op, arg1t, arg2t);

    zvmgetarg2mov(op, arg1t, arg2t, dptr, uint8_t);
    *dptr = src;
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}

void
zvmopmovw(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint16_t     *dptr;
    uint16_t      src = zvmgetarg1mov(op, arg1t, arg2t);

    zvmgetarg2mov(op, arg1t, arg2t, dptr, uint16_t);
    *dptr = src;
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}

#if (!ZVM32BIT)
void
zvmopmovq(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    uint_fast8_t  arg2t = op->arg2t;
    uint64_t     *dptr;
    uint64_t      src = zvmgetarg1mov(op, arg1t, arg2t);
    uint64_t      dest = zvmgetarg2mov(op, arg1t, arg2t, dptr, uint64_t);

    *dptr = src;
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}
#endif

void
zvmopcall(struct zvmopcode *op)
{
    uint_fast8_t arg1t = op->arg1t;
    zasword_t   *dptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]];
    zasword_t    dest = zvmgetarg1(op, arg1t);
    zasword_t    src;
    long         l;
    
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    for (l = 0 ; l < ZVMNREG ; l++) {
        src = zvm.regs[l];
        *dptr = src;
        dptr++;
    }
    zvm.cregs[ZVMFPCREG] = zvm.cregs[ZVMSPCREG] - ZVMNREG * sizeof(zasword_t);
    zvm.cregs[ZVMSPCREG] = zvm.cregs[ZVMFPCREG];
    zvm.cregs[ZVMPCCREG] = dest;

    return;
}

void
zvmopenter(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = op->arg1t;
    zasword_t    *dptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]] - sizeof(zasword_t);
    uint_fast8_t  ofs = zvmgetarg1(op, arg1t);

    *dptr = zvm.cregs[ZVMFPCREG];
    zvm.cregs[ZVMSPCREG] -= ofs + sizeof(zasword_t);
    zvm.cregs[ZVMPCCREG] += op->size << 2;

    return;
}

void
zvmopleave(struct zvmopcode *op)
{
    zasword_t fp = *(zasword_t *)&zvm.physmem[zvm.cregs[ZVMFPCREG]];

    zvm.cregs[ZVMFPCREG] = fp;
    zvm.cregs[ZVMSPCREG] = fp - sizeof(zasword_t);
    zvm.cregs[ZVMPCCREG] += op->size << 2;

    return;
}

void
zvmopret(struct zvmopcode *op)
{
    zasword_t *fptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMFPCREG]];
    zasword_t  fp = *fptr--;
    zasword_t  pc = *fptr--;
    zasword_t *sptr = fptr;
    long       l;

    for (l = 0 ; l < ZVMNREG ; l++) {
        zvm.regs[l] = *sptr--;
    }
    zvm.cregs[ZVMFPCREG] = fp;
    zvm.cregs[ZVMSPCREG] = fp;
    zvm.cregs[ZVMPCCREG] = pc;

    return;
}

void
zvmoplmsw(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *dptr;

    zvmgetarg(op, arg1t, dptr);
    *dptr = zvm.cregs[ZVMMSWCREG];
    zvm.cregs[ZVMPCCREG] += op->size << 2;
}

void zvmopsmsw(struct zvmopcode *op)
{
    uint_fast8_t  arg1t = ZVMARGREG;
    zasword_t    *sptr;

    zvmgetarg(op, arg1t, sptr);
    zvm.cregs[ZVMMSWCREG] = *sptr;
    zvm.cregs[ZVMPCCREG] += op->size << 2;

    return;
}

void
zvmopreset(struct zvmopcode *op)
{
    zasword_t *dptr = zvm.regs;
    long       l;

    zvm.cregs[ZVMMSWCREG] = 0;
    zvm.cregs[ZVMFPCREG] = 0;
    zvm.cregs[ZVMSPCREG] = zvm.memsize;
    zvm.cregs[ZVMPCCREG] = ZVMTEXTBASE;
    for (l = 0 ; l < ZVMNREG ; l++) {
        *dptr++ = 0;
    }

    return;
}

void
zvmophlt(struct zvmopcode *op)
{
    zvm.shutdown = 1;

    return;
}

