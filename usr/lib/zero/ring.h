#ifndef __ZERO_RING_H__
#define __ZERO_RING_H__

/* RING_TYPE    - type of items in ring buffer */
/* RING_INVAL   - invalid/non-present item value */

struct ringbuf {
    volatile long  lk;
    RING_TYPE     *base;
    RING_TYPE     *lim;
    RING_TYPE     *outptr;
    RING_TYPE     *inptr;
};

/* TODO: ring buffer initialisation */

static __inline__ RING_TYPE
ringget(struct ringbuf *buf)
{
    RING_TYPE item = RING_INVAL;

    mtxlk(&buf->lk);
    if (buf->inptr == buf->lim) {
        buf->inptr = buf->base;
    }
    if (buf->inptr < buf->outptr) {
        item = *buf->inptr++;
    }
    mtxunlk(&buf->lk);

    return item;
}

static __inline__ RING_TYPE
ringput(struct ringbuf *buf, RING_TYPE val)
{
    RING_TYPE item = RING_INVAL;

    mtxlk(&buf->lk);
    if (buf->outptr == buf->lim) {
        buf->outptr = buf->base;
    }
    if (buf->outptr < buf->inptr) {
        *buf->outptr++ = val;
        item = val;
    }
    mtxunlk(&buf->lk);

    return item;
}

#endif

