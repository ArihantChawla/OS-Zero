#include <stdlib.h>
#include <limits.h>
#include <zero/mtx.h>
#include <zero/maplk.h>

zeromaplk *
maplkinit(zeromaplk *maplk, long n)
{
    uintptr_t  own;
    void      *map;
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
        map->mtx = ZEROMTXINITVAL;
        mtxlk(&map->mtx);
        maplk = map;
    }
    map = calloc(n, sizeof(long) / CHAR_BIT);
    if (!map && (own)) {
        free(maplk);

        return NULL;
    }
    maplk->n = n;
    maplk->bits = map;
    mtxunlk(&maplk->bits);

    return maplk;
}

