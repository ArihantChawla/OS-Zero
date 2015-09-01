#include <stdlib.h>
#include <limits.h>
#include <zero/mtx.h>
#include <zero/lkmap.h>

long
lkmapinit(long n, zerolkmap *lkmap)
{
    uintptr_t  own = !(uintptr_t)lkmap;
    void      *map = lkmap;
    void      *bits;

    if (own) {
        if (!mtxtrylk(&lkmap->lk)) {

            return 0;
        }
    }
    if (!map) {
        map = malloc(sizeof(zerolkmap));
        if (!map) {

            return NULL;
        }
        lkmap = map;
    } else {
        mtxlk(&lkmap->lk);
    }
    map = calloc(n, sizeof(long) / CHAR_BIT);
    if (!map && (own)) {
        free(lkmap);

        return NULL;
    }
    lkmap->bits = map;

    return lkmap;
}

