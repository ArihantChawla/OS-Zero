#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <zero/mtx.h>
#include <zpu/zpu.h>

static zpuinstfunc   *zpuinstfunctab[ZPUNINST];
static uint8_t       *zpucore;
static volatile long  zpulk;

void
zpuinit(void)
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
zpuruninst(struct zpuctx *ctx)
{
    long          opadr = ctx->pc;
    struct zpuop *op = (struct zpuop *)&zpucore[opadr];
    uint32_t      pc = OP_INVAL;
    long          inst = op->inst;
    int64_t       src;
    int64_t       dest;
    zpuinstfunc  *func = zpuinstfunctab[inst];

    if (func) {
        func(src, dest);
    }

    return pc;
}

int
main(int argc, char *argv[])
{
    zpuinit();

    exit(0);
}

