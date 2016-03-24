/* FIXME: see under MARK to fix more stuff... */

/* zero virtual machine instruction set implementation */

#include <zpm/conf.h>
#include <limits.h>
#include <stdint.h>
#include <zero/trix.h>
#include <zpm/zpm.h>

/* TODO: there be bugs here; proof-read the whole file and fix it... :) */

void
zpmopnot(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    dest = ~dest;
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopand(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    dest &= src;
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopor(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    dest |= src;
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopxor(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    dest ^= src;
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopshr(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);
    zasword_t  fill = ~((zasword_t)0) >> src;

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    dest >>= src;
    dest &= fill;
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopsar(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);
    zasword_t  mask = ~(zasword_t)0;
#if (ZPM32BIT)
    zasword_t  fill = (((dest) & (1 << 31))
                       ? (mask >> (32 - src))
                       : 0);
#else
    zasword_t  fill = (((dest) & (INT64_C(1) << 63))
                       ? (mask >> (64 - src))
                       : 0);
#endif

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    fill = -fill << (CHAR_BIT * sizeof(zasword_t) - src);
    dest >>= src;
    dest |= fill;
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopshl(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    dest <<= src;
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopror(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);
    zasword_t  mask = ~((zasword_t)0) >> (CHAR_BIT * sizeof(zasword_t) - src);
    zasword_t  bits = dest & mask;
    zasword_t  cf = dest & ((zasword_t)1 << (src - 1));

    bits <<= CHAR_BIT * sizeof(zasword_t) - src;
    dest >>= src;
    dest |= bits;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    if (cf) {
        zpm.msw_reg |= ZPM_MSW_CF;
    }
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmoprol(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);
    zasword_t  mask = (~((zasword_t)0) << (CHAR_BIT * sizeof(zasword_t) - src));
    zasword_t  bits = dest & mask;
    zasword_t  cf = dest & ((zasword_t)1 << (src - 1));

    bits >>= CHAR_BIT * sizeof(zasword_t) - src;
    dest <<= src;
    dest |= bits;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    if (cf) {
        zpm.msw_reg |= ZPM_MSW_CF;
    }
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopinc(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    dest++;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    if (!dest) {
        zpm.msw_reg |= ZPM_MSW_OF;
    }
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopdec(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    dest--;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopadd(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);
    zasword_t  res = src + dest;

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(res);
    if (res < dest) {
        zpm.msw_reg |= ZPM_MSW_OF;
    }
    *dptr = res;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopsub(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    dest -= src;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopcmp(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    dest -= src;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    if (src > dest) {
        zpm.msw_reg |= ZPM_MSW_OF;
    }
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopmul(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);
    int64_t    res = dest * src;

    dest *= src;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
#if (ZPM32BIT)
    if (res & INT64_C(0xffffffff00000000)) {
        zpm.msw_reg |= ZPM_MSW_OF;
    }
#else
#error fix integer overflow detection in zpmopmul
#endif
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopdiv(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    dest /= src;
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    if (src > dest) {
        zpm.msw_reg |= ZPM_MSW_OF;
    }
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopmod(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t *dptr;
    zasword_t  dest = zpmgetdestarg(&zpm, op, dptr);

    if (powerof2(src)) {
        dest &= src - 1;
    } else {
        dest %= src;
    }
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    zpmsetzf(dest);
    if (src > dest) {
        zpm.msw_reg |= ZPM_MSW_OF;
    }
    *dptr = dest;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopjmp(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    zpm.pc_reg = dest;

    return;
}

void
zpmopbz(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (zpm.msw_reg & ZPM_MSW_ZF) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopbnz(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (!(zpm.msw_reg & ZPM_MSW_ZF)) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopblt(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (!(zpm.msw_reg & ZPM_MSW_OF)) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopble(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (!(zpm.msw_reg & ZPM_MSW_OF) || !(zpm.msw_reg & ZPM_MSW_ZF)) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopbgt(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (zpm.msw_reg & ZPM_MSW_OF) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopbge(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if ((zpm.msw_reg & ZPM_MSW_OF) || (zpm.msw_reg & ZPM_MSW_ZF)) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopbo(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (zpm.msw_reg & ZPM_MSW_OF) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopbno(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (!(zpm.msw_reg & ZPM_MSW_OF)) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopbc(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (zpm.msw_reg & ZPM_MSW_CF) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmopbnc(struct zpmopcode *op)
{
    zasword_t dest = zpmgetjmparg(&zpm, op);

    if (!(zpm.msw_reg & ZPM_MSW_CF)) {
        zpm.pc_reg = dest;
    } else {
        zpm.pc_reg += sizeof(struct zpmopcode);
    }

    return;
}

void
zpmoppop(struct zpmopcode *op)
{
    zasword_t   src = *(zasword_t *)&zpm.physmem[zpm.sp_reg];
    zasword_t   sp = zpm.sp_reg;
    zasworad_t *dptr = &zpm.physmem[(op)->reg1];

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    sp += sizeof(zasword_t);
    zpmsetzf(src);
    *dptr = src;
    zpm.sp_reg = sp;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmoppush(struct zpmopcode *op)
{
    zasword_t  src = zpmgetsrcarg(&zpm, op);
    zasword_t  sp = zpm.sp_reg;
    zasword_t *dptr;

    sp -= sizeof(zasword_t);
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    dptr = (zasword_t *)&zpm.physmem[sp];
    zpmsetzf(src);
    *dptr = src;
    zpm.sp_reg = sp;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmoppusha(struct zpmopcode *op)
{
    zasword_t  src;
    zasword_t *dptr = (zasword_t *)&zpm.physmem[zpm.sp_reg];
    long       l;

    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    for (l = 0 ; l < ZPMNREG ; l++) {
        --dptr;
        src = zpm.regs[l];
        *dptr = src;
    }
    zpm.sp_reg -= ZPMNREG * sizeof(zasword_t);
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

/* MARK */

void
zpmopldr(struct zpmopcode *op)
{
    zasword_t *sptr = zpmgetldrsrcptr(&zpm, op);
    zasword_t  dest = zpmgetdestptr(&zpm, op);
}

void
zpmopmovl(struct zpmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint32_t  *dptr;
    uint32_t   src = zpmgetsrcargmov(op, arg1t, arg2t);

    zpmgetarg2mov(op, arg1t, arg2t, dptr, uint32_t);
    *dptr = src;
    zpm.pc_reg += sizeof(struct zpmopcode);
}

void
zpmopmovb(struct zpmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint8_t   *dptr;
    uint8_t    src = zpmgetsrcargmov(op, arg1t, arg2t);

    zpmgetarg2mov(op, arg1t, arg2t, dptr, uint8_t);
    *dptr = src;
    zpm.pc_reg += sizeof(struct zpmopcode);
}

void
zpmopmovw(struct zpmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint16_t  *dptr;
    uint16_t   src = zpmgetsrcargmov(op, arg1t, arg2t);

    zpmgetarg2mov(op, arg1t, arg2t, dptr, uint16_t);
    *dptr = src;
    zpm.pc_reg += sizeof(struct zpmopcode);
}

#if (!ZPM32BIT)
void
zpmopmovq(struct zpmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t  arg2t = op->arg2t;
    uint64_t  *dptr;
    uint64_t   src = zpmgetsrcargmov(op, arg1t, arg2t);
    uint64_t   dest = zpmgetarg2mov(op, arg1t, arg2t, dptr, uint64_t);

    *dptr = src;
    zpm.pc_reg += sizeof(struct zpmopcode);
}
#endif

void
zpmopcall(struct zpmopcode *op)
{
    zasword_t  arg1 = op->reg1;
    zasword_t *dptr = (zasword_t *)&zpm.physmem[zpm.sp_reg];
    zasword_t  dest = zpmgetsrcarg(op, arg1t);
    zasword_t  src;
    long       l;
    
    zpm.msw_reg &= ~(ZPM_MSW_ZF | ZPM_MSW_OF | ZPM_MSW_CF);
    for (l = 0 ; l < ZPMNREG ; l++) {
        src = zpm.regs[l];
        *dptr = src;
        dptr++;
    }
    zpm.fp_reg = zpm.sp_reg - ZPMNREG * sizeof(zasword_t);
    zpm.sp_reg = zpm.fp_reg;
    zpm.pc_reg = dest;

    return;
}

void
zpmopenter(struct zpmopcode *op)
{
    zasword_t  arg1t = op->reg1;
    zasword_t *dptr = (zasword_t *)&zpm.physmem[zpm.sp_reg] - sizeof(zasword_t);
    zasword_t  ofs = zpmgetsrcarg(op, reg1);

    *dptr = zpm.fp_reg;
    zpm.sp_reg -= ofs + sizeof(zasword_t);
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopleave(struct zpmopcode *op)
{
    zasword_t fp = *(zasword_t *)&zpm.physmem[zpm.fp_reg];

    zpm.fp_reg = fp;
    zpm.sp_reg = fp - sizeof(zasword_t);
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopret(struct zpmopcode *op)
{
    zasword_t *fptr = (zasword_t *)&zpm.physmem[zpm.fp_reg];
    zasword_t  fp = *fptr--;
    zasword_t  pc = *fptr--;
    zasword_t *sptr = fptr;
    long       l;

    for (l = 0 ; l < ZPMNREG ; l++) {
        zpm.regs[l] = *sptr--;
    }
    zpm.fp_reg = fp;
    zpm.sp_reg = fp;
    zpm.pc_reg = pc;

    return;
}

void
zpmoplmsw(struct zpmopcode *op)
{
    zasword_t  reg1 = ZPMARGREG;
    zasword_t *dptr;

    zvgetarg(op, reg1, dptr);
    *dptr = zpm.msw_reg;
    zpm.pc_reg += sizeof(struct zpmopcode);
}

void zpmopsmsw(struct zpmopcode *op)
{
    zasword_t  reg1 = ZPMARGREG;
    zasword_t *sptr;

    zpmgetarg(op, reg1, sptr);
    zpm.msw_reg = *sptr;
    zpm.pc_reg += sizeof(struct zpmopcode);

    return;
}

void
zpmopreset(struct zpmopcode *op)
{
    zasword_t *dptr = zpm.regs;
    long       l;

    zpm.msw_reg = 0;
    zpm.fp_reg = 0;
    zpm.sp_reg = zpm.memsize;
    zpm.pc_reg = ZPMTEXTBASE;
    for (l = 0 ; l < ZPMNREG ; l++) {
        *dptr++ = 0;
    }

    return;
}

void
zpmophlt(struct zpmopcode *op)
{
    zpm.shutdown = 1;

    return;
}

