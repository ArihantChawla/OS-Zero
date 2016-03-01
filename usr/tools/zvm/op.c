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
    zasword_t  src = op->reg1;
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest = ~dest;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopand(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest &= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopor(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest |= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopxor(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest ^= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopshr(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);
    zasword_t  fill = ~((zasword_t)0) >> src;

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest >>= src;
    dest &= fill;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopsar(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);
    zasword_t  mask = ~(zasword_t)0;
#if (ZVM32BIT)
    zasword_t  fill = (((dest) & (1 << 31))
                       ? (mask >> (32 - src))
                       : 0);
#else
    zasword_t  fill = (((dest) & (INT64_C(1) << 63))
                       ? (mask >> (64 - src))
                       : 0);
#endif

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    fill = -fill << (CHAR_BIT * sizeof(zasword_t) - src);
    dest >>= src;
    dest |= fill;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopshl(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dest <<= src;
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopror(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);
    zasword_t  mask = ~((zasword_t)0) >> (CHAR_BIT * sizeof(zasword_t) - src);
    zasword_t  bits = dest & mask;
    zasword_t  cf = dest & ((zasword_t)1 << (src - 1));

    bits <<= CHAR_BIT * sizeof(zasword_t) - src;
    dest >>= src;
    dest |= bits;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (cf) {
        zvm.cregs[ZVMMSWCREG] |= ZVMCF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmoprol(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);
    zasword_t  mask = (~((zasword_t)0) << (CHAR_BIT * sizeof(zasword_t) - src));
    zasword_t  bits = dest & mask;
    zasword_t  cf = dest & ((zasword_t)1 << (src - 1));

    bits >>= CHAR_BIT * sizeof(zasword_t) - src;
    dest <<= src;
    dest |= bits;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (cf) {
        zvm.cregs[ZVMMSWCREG] |= ZVMCF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopinc(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    dest++;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (!dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopdec(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    dest--;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopadd(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);
    zasword_t  res = src + dest;

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(res);
    if (res < dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = res;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopsub(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    dest -= src;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopcmp(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    dest -= src;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopmul(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);
    int64_t    res = dest * src;

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
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopdiv(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

    dest /= src;
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    zvmsetzf(dest);
    if (src > dest) {
        zvm.cregs[ZVMMSWCREG] |= ZVMOF;
    }
    *dptr = dest;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopmod(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t *dptr;
    zasword_t  dest = zvmgetdestarg(&zvm, op, dptr);

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
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopjmp(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    zvm.cregs[ZVMPCCREG] = dest;

    return;
}

void
zvmopbz(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (zvm.cregs[ZVMMSWCREG] & ZVMZF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopbnz(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMZF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopblt(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMOF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopble(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMOF) || !(zvm.cregs[ZVMMSWCREG] & ZVMZF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopbgt(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (zvm.cregs[ZVMMSWCREG] & ZVMOF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopbge(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if ((zvm.cregs[ZVMMSWCREG] & ZVMOF) || (zvm.cregs[ZVMMSWCREG] & ZVMZF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopbo(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (zvm.cregs[ZVMMSWCREG] & ZVMOF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopbno(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMOF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopbc(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (zvm.cregs[ZVMMSWCREG] & ZVMCF) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmopbnc(struct zvmopcode *op)
{
    zasword_t dest = zvmgetjmparg(&zvm, op);

    if (!(zvm.cregs[ZVMMSWCREG] & ZVMCF)) {
        zvm.cregs[ZVMPCCREG] = dest;
    } else {
        zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
    }

    return;
}

void
zvmoppop(struct zvmopcode *op)
{
    zasword_t   src = *(zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]];
    zasword_t   sp = zvm.cregs[ZVMSPCREG];
    zasworad_t *dptr = &zvm.physmem[(op)->reg1];

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    sp += sizeof(zasword_t);
    zvmsetzf(src);
    *dptr = src;
    zvm.cregs[ZVMSPCREG] = sp;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmoppush(struct zvmopcode *op)
{
    zasword_t  src = zvmgetarg1(&zvm, op);
    zasword_t  sp = zvm.cregs[ZVMSPCREG];
    zasword_t *dptr;

    sp -= sizeof(zasword_t);
    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    dptr = (zasword_t *)&zvm.physmem[sp];
    zvmsetzf(src);
    *dptr = src;
    zvm.cregs[ZVMSPCREG] = sp;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmoppusha(struct zvmopcode *op)
{
    zasword_t  src;
    zasword_t *dptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]];
    long       l;

    zvm.cregs[ZVMMSWCREG] &= ~(ZVMZF | ZVMOF | ZVMCF);
    for (l = 0 ; l < ZVMNREG ; l++) {
        --dptr;
        src = zvm.regs[l];
        *dptr = src;
    }
    zvm.cregs[ZVMSPCREG] -= ZVMNREG * sizeof(zasword_t);
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

/* MARK */

void
zvmopmovl(struct zvmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint32_t     *dptr;
    uint32_t      src = zvmgetarg1mov(op, arg1t, arg2t);

    zvmgetarg2mov(op, arg1t, arg2t, dptr, uint32_t);
    *dptr = src;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
}

void
zvmopmovb(struct zvmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint8_t      *dptr;
    uint8_t       src = zvmgetarg1mov(op, arg1t, arg2t);

    zvmgetarg2mov(op, arg1t, arg2t, dptr, uint8_t);
    *dptr = src;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
}

void
zvmopmovw(struct zvmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint16_t     *dptr;
    uint16_t      src = zvmgetarg1mov(op, arg1t, arg2t);

    zvmgetarg2mov(op, arg1t, arg2t, dptr, uint16_t);
    *dptr = src;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
}

#if (!ZVM32BIT)
void
zvmopmovq(struct zvmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint64_t     *dptr;
    uint64_t      src = zvmgetarg1mov(op, arg1t, arg2t);
    uint64_t      dest = zvmgetarg2mov(op, arg1t, arg2t, dptr, uint64_t);

    *dptr = src;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
}
#endif

void
zvmopcall(struct zvmopcode *op)
{
    zasword_t arg1 = op->reg1;
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
    zasword_t  arg1t = op->reg1;
    zasword_t    *dptr = (zasword_t *)&zvm.physmem[zvm.cregs[ZVMSPCREG]] - sizeof(zasword_t);
    zasword_t  ofs = zvmgetarg1(op, reg1);

    *dptr = zvm.cregs[ZVMFPCREG];
    zvm.cregs[ZVMSPCREG] -= ofs + sizeof(zasword_t);
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

    return;
}

void
zvmopleave(struct zvmopcode *op)
{
    zasword_t fp = *(zasword_t *)&zvm.physmem[zvm.cregs[ZVMFPCREG]];

    zvm.cregs[ZVMFPCREG] = fp;
    zvm.cregs[ZVMSPCREG] = fp - sizeof(zasword_t);
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

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
    zasword_t  reg1 = ZVMARGREG;
    zasword_t    *dptr;

    zvgetarg(op, reg1, dptr);
    *dptr = zvm.cregs[ZVMMSWCREG];
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);
}

void zvmopsmsw(struct zvmopcode *op)
{
    zasword_t  reg1 = ZVMARGREG;
    zasword_t    *sptr;

    zvmgetarg(op, reg1, sptr);
    zvm.cregs[ZVMMSWCREG] = *sptr;
    zvm.cregs[ZVMPCCREG] += sizeof(struct zvmopcode);

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

