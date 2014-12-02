/* zero c library binary-search routines */

#include <stddef.h>
#include <stdint.h>

void *
bsearch(const void *key, const void *base,
        size_t nitems, size_t itemsize,
        int (*cmp)(const void *, const void *))
{
    const void *ptr;
    size_t      l;
    size_t      lim;
    size_t      ndx;
    int         cmpres;

    l = 0;
    lim = nitems;
    while (l < lim) {
        ndx = (l + lim) >> 1;
        ptr = (void *)((uint8_t *)base + (ndx * itemsize));
        cmpres = cmp(key, ptr);
        if (cmpres < 0) {
            lim = ndx;
        } else if (cmpres > 0) {
            l = ndx + 1;
        } else {

            return (void *)ptr;
        }
    }

    return NULL;
}

