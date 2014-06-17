#include <stdint.h>
#include <zero/mtx.h>
#include <zpu/zpu.h>

static uint8_t       *zpucore;
static volatile long  zpulk;
static long           zpuflags;

long
zpuinit(void)
{
    void *ptr;

    mtxlk(&zpulk);
    if (!(zpuflags & ZPUINIT)) {
        zpucore = malloc(ZPUCORESIZE);
    }
    mtxunlk(&zpulk);
}

long
zpuruninst(struct zpu *zpu)
{
    ;
}

