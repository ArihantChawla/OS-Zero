/* internal/test routines for the zero/mem.c module */

#if (MEMTEST)

#include <stdlib.h>
#include <stdio.h>
#include <zero/mem.h>

long
_memchkptr(struct membuf *buf, MEMPTR_T ptr)
{
    MEMUWORD_T nblk = memgetbufnblk(buf);
    MEMUWORD_T type = memgetbuftype(buf);
    MEMUWORD_T slot = memgetbufslot(buf);
    MEMUWORD_T sz = membufblksize(buf);
    long       fail = 0;
    MEMPTR_T   lim;

    if (type == MEMSMALLBUF) {
        lim = buf->base + memsmallbufsize(slot);
    } else if (type == MEMPAGEBUF) {
        lim = buf->base + mempagebufsize(slot, nblk);
    } else {
        lim = buf->base + membigbufsize(slot, nblk);
    }
    if (buf->base + nblk * sz >= lim) {
        fprintf(stderr, "pointer out of bounds: %p\n", ptr);
        fail++;
    }
    if (fail) {
        fprintf(stderr, "BUF\n");
        fprintf(stderr, "---\n");
        fprintf(stderr, "nblk:\t%lx\n", nblk);
        fprintf(stderr, "type:\t%lx\n", type);
        fprintf(stderr, "slot:\t%lx\n", slot);
        fprintf(stderr, "bufsz:\t%lx\n", sz);

        abort();
    }

    return 0;
}

long
_memchkbuf(struct membuf *buf)
{
    MEMUWORD_T nblk = memgetbufnblk(buf);
    MEMUWORD_T nfree = memgetbufnfree(buf);
    MEMUWORD_T type = memgetbuftype(buf);
    MEMUWORD_T slot = memgetbufslot(buf);
    MEMUWORD_T sz = membufblksize(buf);
    long       fail = 0;
    MEMPTR_T   lim;

    if (nfree > nblk) {
        fprintf(stderr, "nfree > nblk\n");
        fail++;
    }
    if (type == MEMSMALLBUF) {
        lim = buf->base + memsmallbufsize(slot);
    } else if (type == MEMPAGEBUF) {
        lim = buf->base + mempagebufsize(slot, nblk);
    } else {
        lim = buf->base + membigbufsize(slot, nblk);
    }
    if (buf->base + nblk * sz >= lim) {
        fprintf(stderr, "bufsize too small\n");
        fail++;
    }
    if (fail) {
        fprintf(stderr, "BUF\n");
        fprintf(stderr, "---\n");
        fprintf(stderr, "nblk:\t%lx\n", nblk);
        fprintf(stderr, "nfree:\t%lx\n", nfree);
        fprintf(stderr, "type:\t%lx\n", type);
        fprintf(stderr, "slot:\t%lx\n", slot);
        fprintf(stderr, "bufsz:\t%lx\n", sz);

        abort();
    }

    return 0;
}

#endif

