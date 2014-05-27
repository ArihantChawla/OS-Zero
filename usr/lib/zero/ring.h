#ifndef __ZERO_RING_H__
#define __ZERO_RING_H__

#include <stdint.h>

/* RING_TYPE    - type of items in ring buffer */
/* RING_INVAL   - invalid/non-present item value */

struct ringbuf {
    volatile long  lk;
    RING_TYPE     *base;
    RING_TYPE     *lim;
    RING_TYPE     *inptr;
    RING_TYPE     *outptr;
};

/*
 * initialise ring buffer
 * - if base == NULL, allocate the data buffer
 */
static __inline__ long
ringinit(struct ringbuf *buf, void *base, long n)
{
    long retval = 0;

    if (!base) {
#if (__KERNEL__)
        base = kmalloc(n * sizeof(RING_TYPE));
#else
        base = malloc(n * sizeof(RING_TYPE));
#endif
        if (base) {
            retval = 1;
        }
    } else {
        retval = 1;
    }
    if (base) {
        buf->base = base;
        buf->lim = (uint8_t *)base + n * sizeof(RING_TYPE);
        buf->inptr = buf->base;
        buf->outptr = buf->base;
    }

    return retval;
}

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

#endif /* __ZERO_RING_H__ */

