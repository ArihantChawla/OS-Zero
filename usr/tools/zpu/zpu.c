#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zpu/zpu.h>

static zpuinstfunc   *zpuinstfunctab[ZPUNINST] ALIGNED(PAGESIZE);
static struct zpu zpu ALIGNED(PAGESIZE);

#define zpusetinst(id, func) (zpuinstfunctab[(id)] = (func))

void
zpuinitcore(struct zpu *zpu)
{
    void *ptr;

    ptr = malloc(ZPUCORESIZE);
    if (!ptr) {
        fprintf(stderr, "ZPU: out of memory (core)\n");

        exit(1);
    }
    zpu->core = ptr;

    return;
}

ZPUOPRET
zpuopnot(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = sreg;
    int64_t src = zpu->ctx.regs[sreg] & INT64_C(0xffffffff);
    int64_t dest = ~src;

    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopand(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t src = zpu->ctx.regs[sreg] & INT64_C(0xffffffff);
    int64_t dest = zpu->ctx.regs[dreg] & INT64_C(0xffffffff);

    dest &= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t src = zpu->ctx.regs[sreg] & INT64_C(0xffffffff);
    int64_t dest = zpu->ctx.regs[dreg] & INT64_C(0xffffffff);

    dest |= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopxor(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t src = zpu->ctx.regs[sreg] & INT64_C(0xffffffff);
    int64_t dest = zpu->ctx.regs[dreg] & INT64_C(0xffffffff);

    dest ^= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopshr(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t src = zpu->ctx.regs[sreg] & INT64_C(0xffffffff);
    int64_t dest = zpu->ctx.regs[dreg] & INT64_C(0xffffffff);
    int64_t mask = INT64_C(1) << ((64 - src) - 1);

    dest >>= src;
    dest &= mask;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopshra(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t src = zpu->ctx.regs[sreg] & INT64_C(0xffffffff);
    int64_t dest = zpu->ctx.regs[dreg] & INT64_C(0xffffffff);
    int64_t mask = ~INT64_C(0);

    dest >>= src;
    if (src & (INT64_C(1) << 63)) {
        mask -= (INT64_C(1) << src) - 1;
        dest |= mask;
    } else {
        mask -= (INT64_C(1) << (64 - src)) - 1;
        dest &= mask;
    }
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpuopshl(struct zpu *zpu, struct zpuop *op)
{
    int64_t sreg = op->src & ((1 << ZPUNREG) - 1);
    int64_t dreg = op->dest & ((1 << ZPUNREG) - 1);
    int64_t src = zpu->ctx.regs[sreg] & INT64_C(0xffffffff);
    int64_t dest = zpu->ctx.regs[dreg] & INT64_C(0xffffffff);

    dest <<= src;
    zpu->ctx.regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->ctx.pc += 4;

    return;
}

ZPUOPRET
zpulmsw(struct zpu *zpu, struct zpuop *op)
{
    long flg = op->src & ((1 << MSWNBIT) - 1);

    zpu->ctx.msw = flg;
}

ZPUOPRET
zpusmsw(struct zpu *zpu, struct zpuop *op)
{
    ;
}

void
zpuinitinst(struct zpu *zpu)
{
    zpusetinst(OP_NOT, zpuopnot);
    zpu->functab = zpuinstfunctab;
}

void
zpuinit(struct zpu *zpu)
{
    zpuinitinst(zpu);
    zpuinitcore(zpu);

    return;
}

uint32_t
zpuruninst(struct zpu *zpu)
{
    long          opadr = zpu->ctx.pc;
    struct zpuop *op = (struct zpuop *)&zpu->core[opadr];
    uint32_t      pc = OP_INVAL;
    long          inst = op->inst;
    zpuinstfunc  *func = zpu->functab[inst];

    if (func) {
        pc = func(op);
    }

    return pc;
}

int
main(int argc, char *argv[])
{
    uint32_t pc;

    zpuinit(&zpu);

    while (1) {
        pc = zpuruninst(&zpu);
    }

    exit(0);
}

