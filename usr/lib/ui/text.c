#if !(__KERNEL__)
#include <stdlib.h>
#endif
#include <ui/ui.h>
#include <ui/text.h>
#if !defined(FREE)
#define FREE(x) free(x)
#endif
#if !defined(MALLOC)
#define MALLOC(x) malloc(x)
#endif
#if !defined(CALLOC)
#define CALLOC(n, s) calloc(n, s)
#endif
#if !defined(MEMCPY)
#define MEMCPY(dest, src, len) mempcy(dest, src, len)
#endif

void
uifreetextbuf(struct uitextbuf *buf)
{
    TEXT_T          **data = buf->data;
    struct textrend **rend = buf->rend;
    long              nrow = buf->nrow;
    long              n;

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
uiinittextbuf(struct uitextbuf *buf, long nrow, long ncol)
{
    long              ndx;
    TEXT_T          **data;
    struct textrend **rend;
    TEXT_T          *dptr;
    struct textrend  *rptr;

    data = MALLOC(nrow * sizeof(TEXT_T *));
    if (!data) {

        return 0;
    }
    rend = MALLOC(nrow * sizeof(struct textrend *));
    if (!rend) {
        uifreetextbuf(buf);
        
        return 0;
    }
    buf->data = data;
    buf->rend = rend;
    for (ndx = 0 ; ndx < nrow ; ndx++) {
        dptr = CALLOC(ncol, sizeof(TEXT_T));
        if (!dptr) {
            uifreetextbuf(buf);

            return 0;
        }
        data[ndx] = dptr;
        rptr = CALLOC(ncol, sizeof(struct textrend));
        if (!rptr) {
            uifreetextbuf(buf);

            return 0;
        }
        rend[ndx] = rptr;
    }
    buf->nrow = nrow;

    return 1;
}

