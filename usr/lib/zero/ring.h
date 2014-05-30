#ifndef __ZERO_RING_H__
#define __ZERO_RING_H__

#include <stdint.h>
#include <zero/mtx.h>
#if !defined(MALLOC) || !defined(FREE)
#include <stdlib.h>
#endif
#if !defined(MEMCPY)
#include <string.h>
#endif
#if !defined(MALLOC)
#define MALLOC malloc
#endif
#if !defined(FREE)
#define FREE   free
#endif
#if !defined(MEMCPY)
#define MEMCPY memcpy
#endif

/* MALLOC       - function used to allocate data buffer */
/* FREE         - function used to free buffers */
/* MEMCPY       - function used to copy data */
/* RING_TYPE    - type of items in ring buffer */
/* RING_INVAL   - invalid/non-present item value */

struct ringbuf {
    volatile long  lk;
    volatile long  init;
    long           n;
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

    mtxlk(&buf->lk);
    if (buf->init) {

        return;
    }
    if (!base) {
        base = MALLOC(n * sizeof(RING_TYPE));
        if (base) {
            retval++;
        }
    } else {
        retval++;
    }
    if (base) {
        buf->init = 0;
        buf->n = n;
        buf->base = base;
        buf->lim = (uint8_t *)base + n * sizeof(RING_TYPE);
        buf->inptr = buf->base;
        buf->outptr = buf->base;
    }
    mtxunlk(&buf->lk);

    return retval;
}

/* fetch next item from ring buffer */
static __inline__ RING_TYPE
ringget(struct ringbuf *buf)
{
    RING_TYPE item = RING_INVAL;

    mtxlk(&buf->lk);
    if (buf->inptr == buf->lim) {
        buf->inptr = buf->base;
    }
    if (buf->inptr != buf->outptr) {
        item = *buf->inptr++;
    }
    mtxunlk(&buf->lk);

    return item;
}

/* queue item into ring buffer */
static __inline__ RING_TYPE
ringput(struct ringbuf *buf, RING_TYPE val)
{
    RING_TYPE item = RING_INVAL;

    mtxlk(&buf->lk);
    if (buf->outptr == buf->lim) {
        buf->outptr = buf->base;
    }
    if (buf->outptr != buf->inptr) {
        *buf->outptr++ = val;
        item = val;
    } else if (!buf->init) {
        *buf->outptr++ = val;
        item = val;
        buf->init = 1;
    }
    mtxunlk(&buf->lk);

    return item;
}

/* resize data buffer */
void *
ringresize(struct ringbuf *buf, long n)
{
    void *buf = NULL;
    void *src;

    mtxlk(&buf->lk);
    src = buf->base;
    buf = malloc(n * sizeof(RING_TYPE));
    if (buf) {
        if (src && (buf->init)) {
            memcpy(buf, src, buf->n);
        }
        FREE(src);
        buf->base = buf;
        buf->n = n;
    }
    mtxunlk(&buf->lk);

    return buf;
}

#endif /* __ZERO_RING_H__ */

