#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <zero/trix.h>

#include <zed/buf.h>

#define ZEDDEFNROW 128

struct zedrow *
zedallocrow(struct zedrowbuf *rowbuf, size_t size)
{
    ;
}

/* append row to buffer */
struct zedrow *
zedapndrow(struct zedrowbuf *rowbuf, size_t size, void *data)
{
    size_t         nrow;
    void          *ptr;
    struct zedrow *row;

    nrow = rowbuf.nrow;
    if (!nrow) {
        ptr = malloc(ZEDDEFNROW * sizeof(struct zedrowbuf));
        rowbuf.nrowmax = ZEDDEFNROW;
        rowbuf.rows = ptr;
    }
    row = &rowbuf.rows[nrow];
    row->size = size;
    row->data = data;
    nrow++;
    rowbuf.nrow = nrow;
    if (nrow == rowbuf.nrowmax) {
        nrow <<= 1;
        ptr = realloc(rowbuf.rows, nrow * sizeof(struct zedrowbuf));
        if (!ptr) {
            fprintf(stderr, "failed to reallocate row buffer\n");

            return NULL;
        }
        rowbuf.nrowmax = nrow;
        rowbuf.rows = ptr;
    }

    return row;
}

