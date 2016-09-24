#ifndef __KERN_MEM_BITS_MBUF_H__
#define __KERN_MEM_BITS_MBUF_H__

/*
 * REFERENCES
 * ----------
 * https://people.freebsd.org/~bmilekic/code/mb_slab/mbuf.h
 * https://people.freebsd.org/~bmilekic/code/mb_slab/uipc_mbuf.c
 */

#include <stddef.h>
#include <stdint.h>
#include <zero/asm.h>
#include <zero/trix.h>
#include <kern/malloc.h>
#include <kern/util.h>

#define mtod(m, t) ((t)((m)->hdr.data))

/* TODO: perhaps hack a dedicated pool/allocator for memory blocks? */

/* check if it's safe to write to buffer's data region */
#define mbwritable(buf)                                                 \
    (!((buf)->hdr.flg & MBUF_RDONLY_BIT)                                \
     && (!(mbexthdr(buf)->flg & MBUF_EXT_RDONLY)                        \
         || (mbexthdr(buf)->nref > 1)))
/* longword-align buffer data pointer for object of len */
#define mbalignbuf(buf, len)                                            \
    ((buf)->hdr.data += (MBUF_DATA_LEN - (len)) & ~(sizeof(long) - 1))
/* longword-align buffer packet data pointer for object of len */
#define mbalignpkt(buf, len)                                            \
    ((buf)->hdr.data += (MBUF_PKT_LEN - (len)) & ~(sizeof(long) - 1))
/* compute amount of space before current start of data */
#define mbleadspace(buf)                                                \
    (((buf)->hdr.flg & MBUF_EXT_BIT)                                    \
     ? 0                                                                \
     : (((buf)->hdr.flg & MBUF_PKTHDR)                                  \
        ? ((buf)->hdr.data - mbpktbuf(buf))                             \
        : ((buf)->hdr.data - mbdata(buf))))
/* compute amount of space after current end of data */
#define mbtrailspace(buf)                                               \
    (((buf)->hdr.flg & MBUF_EXT_BIT)                                    \
     ? (mbextbuf(buf) + mbextsize(buf) - mbadr(buf))                    \
     : (&mbdata(buf)[MBUF_DATA_LEN] - mbadr(buf) - mblen(buf)))

#endif /* __KERN_MEM_BITS_MBUF_H__ */

