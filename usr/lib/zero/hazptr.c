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

/* scan hazard-pointer table and free items; non-locking, atomic operations */
void
hazptrfree(struct hazptrbin *bin)
{
    void     (*func)(void *) = bin->free;
    HAZPTR_T  *tab = bin->tab;
    void      *ptr;
    long       ndx;
    long       tmp;
    long       lim;

    if (func) {
        lim = bin->nmax;
        while (for ndx = 0 ; ndx < lim ; ndx++) {
            ptr = bin->tab[ndx];
            tmp = (long)ptr & HAZPTR_BUSY_BIT;
            if (!m_cmpsetbit((volatile long *)&bin->tab[ndx],
                             HAZPTR_BUSY_BIT)
                && !hazptrfind(ptr)
                && m_cmpswap((volatile long *)&bin->tab[ndx],
                             tmp,
                             HAZPTR_NONE)) {
                func(ptr);
            }
        }
    }

    return;
}

