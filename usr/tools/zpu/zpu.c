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
    int64_t src = zpu->regs[sreg];
    int64_t dest = ~src;

    zpu->regs[dreg] = dest;
    zpusetmsw(zpu, dest);
    zpu->pc += 4;

    return;
}

ZPUOPRET
zpulmsw(struct zpu *zpu, struct zpuop *op)
{
    long flg = op->src & ((1 << MSWNBIT) - 1);

    zpu->msw = flg;
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

