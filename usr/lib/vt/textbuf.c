#if !(__KERNEL__)
#include <stdlib.h>
#endif
#include <vt/vt.h>
#if !defined(FREE)
#define FREE(x) FREE(x)
#endif
#if !defined(MALLOC)
#define MALLOC(x) malloc(x)
#endif
#if !defined(CALLOC)
#define CALLOC(n, s) calloc(n, s)
#endif
#if !defined(MEMCPY)
#define MALLOC(dest, src, len) mempcy(dest, src, len)
#endif

void
vtfreetextbuf(struct vttextbuf *buf)
{
    int32_t       **data = buf->data;
    struct vtrend **rend = buf->rend;
    long            nrow = buf->nrow;
    long            n;

    if (data) {
        for (n = 0 ; n < nrow ; n++) {
            if (data[n]) {
                FREE(data[n]);
            }
        }
        FREE(data);
    }
    if (rend) {
        for (n = 0 ; n < nrow ; n++) {
            if (rend[n]) {
                FREE(rend[n]);
            }
        }
        FREE(rend);
    }
}

long
vtinittextbuf(struct vttextbuf *buf, long nrow, long ncol)
{
    long            ndx;
    int32_t       **data;
    struct vtrend **rend;
    int32_t        *dptr;
    struct vtrend  *rptr;

    data = MALLOC(nrow * sizeof(int32_t *));
    if (!data) {

        return 0;
    }
    rend = MALLOC(nrow * sizeof(struct vtrend *));
    if (!rend) {
        vtfreetextbuf(buf);
        
        return 0;
    }
    buf->data = data;
    buf->rend = rend;
    for (ndx = 0 ; ndx < nrow ; ndx++) {
        dptr = CALLOC(ncol, sizeof(int32_t));
        if (!dptr) {
            vtfreetextbuf(buf);

            return 0;
        }
        data[ndx] = dptr;
        rptr = CALLOC(ncol, sizeof(struct vtrend));
        if (!rptr) {
            vtfreetextbuf(buf);

            return 0;
        }
        rend[ndx] = rptr;
    }
    buf->nrow = nrow;

    return 1;
}

