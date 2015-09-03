#include <stdlib.h>
#include <limits.h>
#include <zero/mtx.h>
#include <zero/maplk.h>

zeromaplk *
maplkinit(zeromaplk *maplk, long n)
{
    uintptr_t  own;
    zeromaplk *map;
    void      *bits;

    if (maplk) {
        if (!mtxtrylk(&maplk->mtx)) {

            return maplk;
        }
        map = maplk;
    } else {
        map = malloc(sizeof(zeromaplk));
        if (!map) {

            return NULL;
        }
        own = 1;
        map->mtx = MTXINITVAL;
        maplk = map;
        mtxlk(&maplk->mtx);
    }
    bits = calloc(n, sizeof(long) / CHAR_BIT);
    if (!bits && (own)) {
        mtxunlk(&maplk->mtx);
        free(maplk);

        return NULL;
    }
    maplk->nbit = n;
    maplk->bits = bits;
    mtxunlk(&maplk->mtx);

    return maplk;
}

