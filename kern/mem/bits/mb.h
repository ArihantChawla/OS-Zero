#ifndef __KERN_MEM_BITS_MB_H__
#define __KERN_MEM_BITS_MB_H__

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

#define mtod(m, t) ((t)((m)->hdr.buf))

/* TODO: perhaps hack a dedicated pool/allocator for memory blocks? */

/* check if it's safe to write to buffer's data region */
#define mbwritable(buf)                                                 \
    (!((buf)->hdr.flg & MB_RDONLY_BIT)                                  \
     && (!(mbexthdr(buf)->flg & MB_EXT_RDONLY)                          \
         || (mbexthdr(buf)->nref > 1)))
/* longword-align buffer data pointer for object of len */
#define mbalignbuf(buf, len)                                            \
    ((buf)->hdr.buf += (MB_DATA_LEN - (len)) & ~(sizeof(long) - 1))
/* longword-align buffer packet data pointer for object of len */
#define mbalignpkt(buf, len)                                            \
    ((buf)->hdr.buf += (MB_PKT_LEN - (len)) & ~(sizeof(long) - 1))
/* compute amount of space before current start of data */
#define mbleadspace(buf)                                                \
    (((buf)->hdr.flg & MB_EXT_BIT)                                      \
     ? 0                                                                \
     : (((buf)->hdr.flg & MB_PKTHDR)                                    \
        ? ((buf)->hdr.buf - mbpktbuf(buf))                              \
        : ((buf)->hdr.buf - mbdata(buf))))
/* compute amount of space after current end of data */
#define mbtrailspace(buf)                                               \
    (((buf)->hdr.flg & MB_EXT_BIT)                                      \
     ? (mbextbuf(buf) + mbextsize(buf) - mbadr(buf))                    \
     : (&mbdata(buf)[MB_DATA_LEN] - mbadr(buf) - mblen(buf)))

#endif /* __KERN_MEM_BITS_MB_H__ */

