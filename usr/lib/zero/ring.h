#ifndef __ZERO_RING_H__
#define __ZERO_RING_H__

#if !defined(RINGSHAREBUF)
#define RINGSHAREBUF 0
#endif

#include <features.h>
#include <stdint.h>
#include <limits.h>
#if defined(_ZERO_SOURCE) && (RINGSHAREBUF)
#include <sys/syscall.h>
#include <sys/zero/syscall.h>
#include <kern/mem/obj.h>
#endif
#include <zero/cdefs.h>
#include <zero/param.h>
#define ZEROMTX 1
#include <zero/mtx.h>
#include <zero/trix.h>
#if !defined(__KERNEL__)
#if !defined(MALLOC) || !defined(FREE)
#include <stdlib.h>
#endif
#if !defined(MEMCPY)
#include <string.h>
#endif
#if !defined(MALLOC)
#define MALLOC(n)            malloc(n)
#endif
#if !defined(FREE)
#define FREE(ptr)            free(ptr)
#endif
#if !defined(MEMCPY)
#define MEMCPY(dest, src, n) memcpy(dest, src, n)
#endif
#endif /* !defined(__KERNEL__) */

#if (RINGSHAREBUF) && !defined(__KERNEL__)
#if defined(_ISOC11_SOURCE) && (_ISOC11_SOURCE)
#defined VALLOC(n)           aligned_alloc(PAGESIZE, n)
#elif (((defined(_BSD_SOURCE) && (_BSD_SOURCE))                         \
        || (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500             \
                                       || ((defined(_XOPEN_SOURCE_EXTENDED) \
                                            && (_XOPEN_SOURCE_EXTENDED)))))) \
       && !((USEPOSIX200112)                                            \
            || (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600))))     \
#define VALLOC(n)            valloc(n)
#elif (USEPOSIX200112 || (defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600)))
static __inline__ void *
VALLOC(size_t n)
{
    void *ptr;

    if (posix_memalign(&ptr, PAGESIZE, n)) {

        return NULL;
    }

    return ptr;
}
#else
#define VALLOC(n) memalign(PAGESIZE, n)
#endif
#endif /* RINGSHAREBUF && !__KERNEL__ */

/* MALLOC       - function used to allocate data buffer */
/* FREE         - function used to free buffers */
/* MEMCPY       - function used to copy data */
/* RING_ITEM    - type of items in ring buffer */
/* RING_INVAL   - invalid/non-present item value */

/* flg-member bits */
#define RINGBUF_INIT (1 << 0)
struct ringbuf {
    zeromtx    lk;
    long       flg;
    long       nitem;
    RING_ITEM *base;
    RING_ITEM *lim;
    RING_ITEM *inptr;
    RING_ITEM *outptr;
    long       pad;
    /* data buffer */
#if (RINGSHAREBUF)
    uint8_t   *data;
#else
    uint8_t    data[EMPTY];
#endif
} ALIGNED(PAGESIZE);

/*
 * initialise ring buffer
 * - if base == NULL, allocate the data buffer
 */
static __inline__ long
ringinit(void *ptr, void *base, long nitem)
{
    struct ringbuf *buf = ptr; 
    long            retval = 0;

    mtxlk(&buf->lk);
    if (buf->flg & RINGBUF_INIT) {
        mtxunlk(&buf->lk);

        return 1;
    }
    if (!base) {
#if (RINGSHAREBUF)
        base = VALLOC(nitem * sizeof(RING_ITEM));
#else
        base = MALLOC(nitem * sizeof(RING_ITEM));
#endif
        if (base) {
            retval++;
        }
    } else {
        retval++;
    }
    if (base) {
        buf->flg |= RINGBUF_INIT;
        buf->nitem = nitem;
        buf->base = base;
        buf->lim = (RING_ITEM *)((uint8_t *)base + nitem * sizeof(RING_ITEM));
        buf->inptr = base;
        buf->outptr = base;
    }
    mtxunlk(&buf->lk);

    return retval;
}

#if defined(_ZERO_SOURCE) && (RINGSHAREBUF)

static __inline__ long
ringmapbuf(struct ringbuf *buf, long flg, struct perm *perm)
{
    struct memreg mem;

    mem.perm = perm;
    mem.flg = MEM_UMAP | flg;
    mem.adr = buf->base;
    mem.ofs = 0;
    mem.len = buf->nitem * sizeof(RING_ITEM);
//    syscall(SYS_MCTL, MEM_SHMAP, MEM_DUALMAP, &mem);
}

#endif /* defined(_ZERO_SOURCE) && (RINGSHAREBUF) */

/* fetch next item from ring buffer */
static __inline__ RING_ITEM
ringget(struct ringbuf *buf)
{
    RING_ITEM item = RING_INVAL;

    mtxlk(&buf->lk);
    if (buf->outptr == buf->lim) {
        buf->outptr = buf->base;
    }
    if (buf->outptr != buf->inptr) {
        item = *buf->outptr++;
    }
    mtxunlk(&buf->lk);

    return item;
}

/* queue item into ring buffer */
static __inline__ RING_ITEM
ringput(struct ringbuf *buf, RING_ITEM val)
{
    RING_ITEM item = RING_INVAL;

    mtxlk(&buf->lk);
    if (buf->inptr == buf->lim) {
        buf->inptr = buf->base;
    }
    if (buf->inptr != buf->outptr) {
        *buf->inptr++ = val;
        item = val;
    } else if (!(buf->flg & RINGBUF_INIT)) {
        *buf->inptr++ = val;
        item = val;
        buf->flg |= RINGBUF_INIT;
    }
    mtxunlk(&buf->lk);

    return item;
}

/* fetch next items from ring buffer */
static __inline__ long
ringgetmany(struct ringbuf *buf, RING_ITEM *dest, long len)
{
    long nitem;
    long ret;

    mtxlk(&buf->lk);
    if (buf->outptr > buf->inptr) {
        nitem = buf->lim - buf->outptr;
    } else {
        nitem = buf->inptr - buf->outptr;
    }
    nitem = min(len, nitem);
    len -= nitem;
    ret = nitem;
    while (nitem--) {
        *dest++ = *buf->outptr++;
    }
    if (buf->outptr == buf->lim) {
        buf->outptr = buf->base;
    }
    if (buf->outptr != buf->inptr) {
        nitem = buf->inptr - buf->outptr;
        nitem = min(len, nitem);
        ret += nitem;
        while (nitem--) {
            *dest++ = *buf->outptr++;
        }
    }
    mtxunlk(&buf->lk);

    return ret;
}

/* queue items into ring buffer */
static __inline__ long
ringputmany(struct ringbuf *buf, RING_ITEM *src, long len)
{
    long nitem;
    long ret;

    mtxlk(&buf->lk);
    if (buf->inptr < buf->outptr) {
        nitem = buf->outptr - buf->inptr;
    } else {
        nitem = buf->lim - buf->inptr;
    }
    nitem = min(len, nitem);
    len -= nitem;
    ret = nitem;
    while (nitem--) {
        *buf->inptr++ = *src++;
    }
    if (buf->inptr == buf->lim) {
        buf->inptr = buf->base;
    }
    if (buf->inptr != buf->outptr) {
        nitem = buf->outptr - buf->inptr;
        nitem = min(len, nitem);
        ret += nitem;
        while (nitem--) {
            *buf->inptr++ = *src++;
        }
    }
    mtxunlk(&buf->lk);

    return ret;
}

/* fetch next items from ring buffer */
static __inline__ long
ringgetmany32(struct ringbuf *buf, int32_t *dest, long len)
{
    int32_t *ptr;
    int8_t  *i8ptr;
    long     nitem;
    long     ret;
    long     ofs;

    mtxlk(&buf->lk);
    i8ptr = (int8_t *)buf->outptr;
    ofs = sizeof(int32_t) - ((uintptr_t)i8ptr & (sizeof(int32_t) - 1));
    i8ptr += ofs;
    ptr = (int32_t *)i8ptr;
    if ((RING_ITEM *)ptr < buf->inptr) {
        nitem = (int8_t *)buf->inptr - (int8_t *)ptr;
    } else {
        nitem = (int8_t *)buf->lim - (int8_t *)ptr;
    }
    nitem = min(len, nitem >> 2);
    len -= nitem;
    ret = nitem;
    while (nitem--) {
        *dest++ = *ptr++;
    }
    if ((RING_ITEM *)ptr == buf->lim) {
        ptr = (int32_t *)buf->base;
    }
    if ((RING_ITEM *)ptr != buf->inptr) {
        nitem = (int8_t *)buf->inptr - (int8_t *)ptr;
        nitem = min(len, nitem >> 2);
        ret += nitem;
        while (nitem--) {
            *dest++ = *ptr++;
        }
    }
    buf->outptr = (RING_ITEM *)ptr;
    mtxunlk(&buf->lk);

    return ret;
}

/* queue items into ring buffer */
static __inline__ long
ringputmany32(struct ringbuf *buf, int32_t *src, long len)
{
    int32_t *ptr;
    int8_t  *i8ptr;
    long     nitem;
    long     ret;
    long     ofs;

    mtxlk(&buf->lk);
    i8ptr = (int8_t *)buf->inptr;
    ofs = sizeof(int32_t) - ((uintptr_t)i8ptr & (sizeof(int32_t) - 1));
    ptr = (int32_t *)(i8ptr + ofs);
#if defined(RING_PAD_BYTE)
    while (ofs--) {
        *i8ptr++ = RING_PAD_BYTE;
    }
#endif
    if ((RING_ITEM *)ptr < buf->outptr) {
        nitem = (int8_t *)buf->outptr - (int8_t *)ptr;
    } else {
        nitem = (int8_t *)buf->lim - (int8_t *)ptr;
    }
    nitem = min(len, nitem >> 2);
    len -= nitem;
    ret = nitem;
    while (nitem--) {
        *ptr++ = *src++;
    }
    if ((RING_ITEM *)ptr == buf->lim) {
        ptr = (int32_t *)buf->base;
    }
    if ((RING_ITEM *)ptr != buf->outptr) {
        nitem = (int8_t *)buf->outptr - (int8_t *)ptr;
        nitem = min(len, nitem >> 2);
        ret += nitem;
        while (nitem--) {
            *ptr++ = *src++;
        }
    }
    buf->inptr = (RING_ITEM *)ptr;
    mtxunlk(&buf->lk);

    return ret;
}

/* resize data buffer */
static __inline__ void *
ringresize(struct ringbuf *buf, long nitem)
{
    RING_ITEM *ptr = NULL;
    RING_ITEM *src;
    long       nin;
    long       nout;

    mtxlk(&buf->lk);
    if (nitem > buf->nitem) {
        nin = (int8_t *)buf->inptr - (int8_t *)buf->base;
        nout = (int8_t *)buf->outptr - (int8_t *)buf->base;
        src = buf->base;
        ptr = MALLOC(nitem * sizeof(RING_ITEM));
        if (ptr) {
            if (src && (buf->flg & RINGBUF_INIT)) {
                MEMCPY(ptr, src, buf->nitem);
            }
            FREE(src);
            buf->nitem = nitem;
            buf->base = ptr;
            buf->lim = (RING_ITEM *)((int8_t *)ptr
                                     + nitem * sizeof(RING_ITEM));
            buf->inptr = (RING_ITEM *)((int8_t *)ptr + nin);
            buf->outptr = (RING_ITEM *)((int8_t *)ptr + nout);
        }
    } else {
        ptr = buf->base;
    }
    mtxunlk(&buf->lk);

    return ptr;
}

#endif /* __ZERO_RING_H__ */

