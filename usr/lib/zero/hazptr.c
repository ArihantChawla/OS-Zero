#if !defined(HAZPTR_MALLOC)
#include <stdlib.h>
#define HAZPTR_MALLOC(sz) malloc(sz)
#endif
#include <zero/hazptr.h>

struct hazptrbin *
hazptrinitbin(struct hazptrbin *bin,
              long n,
              void (*recl)(void *),
              void (*buf)())
{
    void *ptr;
    long  rel = 0;
    long  ndx;
    
    if (!bin) {
        bin = HAZPTR_MALLOC(sizeof(struct hazptrbin));
        rel = 1;
    }
    if (bin) {
        ptr = HAZPTR_MALLOC(n * sizeof(HAZPTR_T));
        if (ptr) {
            bin->flg |= HAZPTR_BIN_INIT;
            bin->cur = n;
            bin->lim = n;
            bin->tab = ptr;
            bin->free = recl;
            bin->buf = buf;

            return bin;
        } else if (rel) {
            free(bin);
        }
    }

    return NULL;
}

