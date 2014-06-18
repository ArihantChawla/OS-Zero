#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <zpu/zpu.h>

static zpuinstfunc   *zpuinstfunctab[ZPUNINST];
struct zpu {
    zpuinstfunc   **functab;
    struct zpuctx   ctx;
    uint8_t        *core;
};
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

int32_t
zpuopnot(struct zpuop *op)
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

