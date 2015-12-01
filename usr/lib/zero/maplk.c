#include <stdlib.h>
#include <limits.h>
#include <zero/mtx.h>
#include <zero/maplk.h>

zeromaplk *
maplkinit(zeromaplk *maplk, unsigned long n)
{
    uintptr_t  own;
    zeromaplk *map;
    void      *bits;

    if (maplk) {
        if (!mtxtrylk(&maplk->lk)) {

            return maplk;
        }
        map = maplk;
    } else {
        map = malloc(sizeof(zeromaplk));
        if (!map) {

            return NULL;
        }
        own = 1;
//        map->lk = MTXINITVAL;
        mtxinit(&map->lk);
        maplk = map;
        mtxlk(&maplk->lk);
    }
    bits = calloc(n, sizeof(long) / CHAR_BIT);
    if (!bits && (own)) {
        mtxunlk(&maplk->lk);
        free(maplk);

        return NULL;
    }
    maplk->nbit = n;
    maplk->bits = bits;
    mtxunlk(&maplk->lk);

    return maplk;
}

