#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zero/mtx.h>
#include <zpu/zpu.h>

static zpuinstfunc   *zpuinstfunctab[ZPUNINST];
struct zpuctx         zpuctx;
static uint8_t       *zpucore;
static volatile long  zpulk;

#define zpusetinst(id, func) (zpuinstfunctab[(id)] = (func))

void
zpuinitcore(void)
{
    void *ptr;

    ptr = malloc(ZPUCORESIZE);
    if (!ptr) {
        fprintf(stderr, "ZPU: out of memory (core)\n");

        exit(1);
    }
    zpucore = ptr;

    return;
}

uint32_t
zpuopnot(struct zpuop *op)
{
    
}

void
zpuinitinst(void)
{
    zpusetinst(OP_NOT, zpuopnot);
}

void
zpuinit(void)
{
    zpuinitcore();
    zpuinitinst();

    return;
}

uint32_t
zpuruninst(struct zpuctx *ctx)
{
    long          opadr = ctx->pc;
    struct zpuop *op = (struct zpuop *)&zpucore[opadr];
    uint32_t      pc = OP_INVAL;
    long          inst = op->inst;
    zpuinstfunc  *func = zpuinstfunctab[inst];

    if (func) {
        pc = func(op);
    }

    return pc;
}

int
main(int argc, char *argv[])
{
    uint32_t pc;

    zpuinit();

    while (1) {
        pc = zpuruninst(&zpuctx);
        zpuctx.pc = pc;
    }

    exit(0);
}

