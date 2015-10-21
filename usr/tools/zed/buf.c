#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <zero/trix.h>

#include <zed/buf.h>

#define ZEDDEFNROW 128

struct zedrowbuf *
zedallocrow(struct zedrowbuf *rowbuf, size_t size)
{
    ;
}

/* append row to buffer */
struct zedrowbuf *
zedapndrow(struct zedrowbuf *rowbuf, size_t size, void *data)
{
    size_t            nrow;
    void             *ptr;
    struct zedrowbuf *row;

    nrow = rowbuf->nrow;
    if (!nrow) {
        ptr = malloc(ZEDDEFNROW * sizeof(struct zedrowbuf));
        rowbuf->nrowmax = ZEDDEFNROW;
        rowbuf->rowtab = ptr;
    }
    row = &rowbuf->rowtab[nrow];
#if 0
    row->size = size;
    row->data = data;
#endif
    nrow++;
    rowbuf->nrow = nrow;
    if (nrow == rowbuf->nrowmax) {
        nrow <<= 1;
        ptr = realloc(rowbuf->rowtab, nrow * sizeof(struct zedrowbuf));
        if (!ptr) {
            fprintf(stderr, "failed to reallocate row buffer\n");

            return NULL;
        }
        rowbuf->nrowmax = nrow;
        rowbuf->rowtab = ptr;
    }

    return row;
}

