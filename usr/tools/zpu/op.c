#include <stdlib.h>
#if (ZPUDEBUG)
#include <stdio.h>
#endif
#include <zpu/conf.h>
#include <zpu/zpu.h>

void
zpuophlt(struct zpu *zpu, struct zpuop *op)
{
#if (ZPUDEBUG)
    fprintf(stderr, "hlt\n");
#endif

    exit(0);
}

void
zpuopnot(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = sreg;
#if (ZPUIREGSIZE == 4)
    int64_t onemask = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & onemask;
#else
    int64_t src = zpu->ctx.regs[sreg];
#endif
    int64_t dest = ~src;

    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

void
zpuopand(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
#if (ZPUIREGSIZE == 4)
    int64_t onemask = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & onemask;
    int64_t dest = zpu->ctx.regs[dreg] & onemask;
#else
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
#endif

    dest &= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

void
zpuopor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
#if (ZPUIREGSIZE == 4)
    int64_t onemask = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & onemask;
    int64_t dest = zpu->ctx.regs[dreg] & onemask;
#else
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
#endif

    dest |= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

void
zpuopxor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
#if (ZPUIREGSIZE == 4)
    int64_t onemask = INT64_C(0xffffffff);
    int64_t src = zpu->ctx.regs[sreg] & onemask;
    int64_t dest = zpu->ctx.regs[dreg] & onemask;
#else
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
#endif

    dest ^= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

void
zpuopshr(struct zpu *zpu, struct zpuop *op)
{
    long    sz = op->argsz;
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    long    cnt = 0;
    long    n = (sz == 7) ? 1 : ((sz == 3) ? 2 : ((sz == 1) ? 4 : 8));
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];;
    int64_t onemask = (INT64_C(1) << (((sz + 1) << 3) - src)) - 1;
    int64_t val = 0;
    int64_t res = 0;

    while (n) {
        val = dest & onemask;
        dest >>= 8;
        val >>= src;
        n--;
        val &= onemask;
        res |= val << cnt;
        cnt += 8;
    }
    zpu->ctx.regs[dreg] = res;
    if (sz == ZPUIREGSIZE) {
        zpusetmsw(zpu, res);
    }
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

void
zpuopsar(struct zpu *zpu, struct zpuop *op)
{
    long    sz = op->argsz;
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    long    cnt = 0;
    long    n = (sz == 7) ? 1 : ((sz == 3) ? 2 : ((sz == 1) ? 4 : 8));
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
    int64_t onemask = (INT64_C(1) << (((sz + 1) << 3) - src)) - 1;
    int64_t mask = ~INT64_C(0);
    int64_t val = 0;
    int64_t res = 0;

#if (ZPUDEBUG)
    fprintf(stderr, "sar\n");
#endif
    while (n) {
        val = dest & onemask;
        dest >>= 8;
        val >>= src;
        n--;
        if (val & (INT64_C(1) << (((sz + 1) << 3) - 1))) {
            mask = onemask - ((INT64_C(1) << src) - 1);
            val |= mask;
        } else {
            mask = (INT64_C(1) << (((sz + 1) << 3) - src)) - 1;
            val &= mask;
        }
        res |= val << cnt;
        cnt += 8;
    }
    zpu->ctx.regs[dreg] = res;
    if (sz == ZPUIREGSIZE) {
        zpusetmsw(zpu, res);
    }
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

void
zpuopshl(struct zpu *zpu, struct zpuop *op)
{
    long    sz = op->argsz;
    int64_t sreg = op->src;
    int64_t dreg = op->dest;
    long    cnt = 0;
    long    n = (sz == 7) ? 1 : ((sz == 3) ? 2 : ((sz == 1) ? 4 : 8));
    int64_t onemask = (INT64_C(1) << ((sz + 1) << 3)) - 1;
    int64_t src = zpu->ctx.regs[sreg];
    int64_t dest = zpu->ctx.regs[dreg];
    int64_t val = 0;
    int64_t res = 0;

    while (n) {
        val = dest & onemask;
        dest >>= 8;
        val <<= src;
        n--;
        val &= onemask;
        res |= val << cnt;
        cnt += 8;
    }
    zpu->ctx.regs[dreg] = res;
    if (sz == ZPUIREGSIZE) {
        zpusetmsw(zpu, res);
    }
    zpu->ctx.regs[ZPUPCREG] += 4;

    return;
}

void
zpuopror(struct zpu *zpu, struct zpuop *op)
{
    
}

void zpuoprol(struct zpu *zpu, struct zpuop *op) { }
void zpuopinc(struct zpu *zpu, struct zpuop *op) { }
void zpuopdec(struct zpu *zpu, struct zpuop *op) { }
void zpuopadd(struct zpu *zpu, struct zpuop *op) { }
void zpuopsub(struct zpu *zpu, struct zpuop *op) { }
void zpuopcmp(struct zpu *zpu, struct zpuop *op) { }
void zpuopmul(struct zpu *zpu, struct zpuop *op) { }
void zpuopdiv(struct zpu *zpu, struct zpuop *op) { }
void zpuopmod(struct zpu *zpu, struct zpuop *op) { }
void zpuopbz(struct zpu *zpu, struct zpuop *op) { }
void zpuopbnz(struct zpu *zpu, struct zpuop *op) { }
void zpuopblt(struct zpu *zpu, struct zpuop *op) { }
void zpuopble(struct zpu *zpu, struct zpuop *op) { }
void zpuopbgt(struct zpu *zpu, struct zpuop *op) { }
void zpuopbge(struct zpu *zpu, struct zpuop *op) { }
void zpuopbo(struct zpu *zpu, struct zpuop *op) { }
void zpuopbno(struct zpu *zpu, struct zpuop *op) { }
void zpuopbc(struct zpu *zpu, struct zpuop *op) { }
void zpuopbnc(struct zpu *zpu, struct zpuop *op) { }
void zpuoppop(struct zpu *zpu, struct zpuop *op) { }
void zpuoppush(struct zpu *zpu, struct zpuop *op) { }
void zpuoppusha(struct zpu *zpu, struct zpuop *op) { }
void zpuopmov(struct zpu *zpu, struct zpuop *op) { }
void zpuopjmp(struct zpu *zpu, struct zpuop *op) { }
void zpuopcall(struct zpu *zpu, struct zpuop *op) { }
void zpuopenter(struct zpu *zpu, struct zpuop *op) { }
void zpuopleave(struct zpu *zpu, struct zpuop *op) { }
void zpuopret(struct zpu *zpu, struct zpuop *op) { }

void
zpuoplmsw(struct zpu *zpu, struct zpuop *op)
{
    long flg = op->src & ((1 << MSWNBIT) - 1);

    zpu->ctx.regs[ZPUMSWREG] |= flg;
}

void
zpuopsmsw(struct zpu *zpu, struct zpuop *op)
{
    ;
}

void zpuopradd(struct zpu *zpu, struct zpuop *op) { }
void zpuoprsub(struct zpu *zpu, struct zpuop *op) { }
void zpuoprmul(struct zpu *zpu, struct zpuop *op) { }
void zpuoprdiv(struct zpu *zpu, struct zpuop *op) { }
void zpuopunpk(struct zpu *zpu, struct zpuop *op) { }
