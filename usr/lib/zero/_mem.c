/* internal/test routines for the zero/mem.c module */

#if (MEMTEST) || (MEMSTAT)

#include <stdlib.h>
#include <stdio.h>
#include <zero/mem.h>

static const char *bufnames[3] = { "SMALL", "PAGE", "BIG" };

#if (MEMSTAT)

extern struct memstat g_memstat;

void
memprintstat(void)
{
    struct memstat *stat = &g_memstat;

    fprintf(stderr, "MEMSTAT\n");
    fprintf(stderr, "--------\n");
    fprintf(stderr, "small\t%llu KB\n",  stat->nbsmall >> 10);
    fprintf(stderr, "page\t%llu KB\n",  stat->nbpage >> 10);
    fprintf(stderr, "big\t%llu KB\n",  stat->nbbig >> 10);
    fprintf(stderr, "heap\t%llu KB\n",  stat->nbheap >> 10);
    fprintf(stderr, "map\t%llu KB\n",  stat->nbmap >> 10);
    fprintf(stderr, "book\t%llu KB\n",  stat->nbbook >> 10);
    fprintf(stderr, "hash\t%llu KB\n",  stat->nbhash >> 10);
}
#endif

void
memprintbuf(struct membuf *buf, const char *func)
{
    MEMWORD_T  nblk = memgetbufnblk(buf);
    MEMWORD_T  type = memgetbuftype(buf);
    MEMWORD_T  slot = memgetbufslot(buf);
    MEMUWORD_T blksz = membufblksize(buf, type, slot);

    if (func) {
        fprintf(stderr, "BUF: (%s)\n", func);
    } else {
        fprintf(stderr, "BUF\n");
    }
    fprintf(stderr, "--------\n");
    fprintf(stderr, "base:\t%p\n", buf->base);
    fprintf(stderr, "nblk:\t%lx\n", nblk);
    fprintf(stderr, "nfree:\t%lx\n", memgetbufnfree(buf));
    fprintf(stderr, "type:\t%lx\n", type);
    fprintf(stderr, "slot:\t%lx\n", slot);
    fprintf(stderr, "blksz:\t%lx\n", blksz);
    fprintf(stderr, "bufsize:\t%lx\n", buf->size);

    return;
}

long
_memchkptr(struct membuf *buf, MEMPTR_T ptr)
{
    MEMWORD_T  nblk = memgetbufnblk(buf);
    MEMWORD_T  nfree = memgetbufnfree(buf);
    MEMWORD_T  type = memgetbuftype(buf);
    MEMWORD_T  slot = memgetbufslot(buf);
    MEMUWORD_T sz = membufblksize(buf, type, slot);
    long       fail = 0;
    MEMPTR_T   lim;

    if (type == MEMSMALLBUF) {
        lim = buf->base + memsmallbufsize(slot, nblk);
    } else if (type == MEMPAGEBUF) {
        lim = buf->base + mempagebufsize(slot, nblk);
    } else {
        lim = buf->base + membigbufsize(slot, nblk);
    }
    if (ptr < buf->base) {
        fprintf(stderr, "pointer out of bounds: %p\n", ptr);
        fail++;
    } else if (ptr + sz > lim) {
        fprintf(stderr, "pointer out of bounds: %p\n", ptr);
        fail++;
    }
    if (fail) {
        fprintf(stderr, "BUF\n");
        fprintf(stderr, "---\n");
        fprintf(stderr, "base:\t%p\n", buf->base);
        fprintf(stderr, "nblk:\t%lx\n", nblk);
        fprintf(stderr, "nfree:\t%lx\n", nfree);
        fprintf(stderr, "type:\t%lx\n", type);
        fprintf(stderr, "slot:\t%lx\n", slot);
        fprintf(stderr, "bufsz:\t%lx\n", sz);

        abort();
    }

    return 0;
}

long
_memchkbuf(struct membuf *buf, MEMWORD_T slot, MEMWORD_T type,
           MEMWORD_T nblk, MEMUWORD_T flg, const char *func)
{
    MEMUWORD_T bflg = memgetbufflg(buf);
    MEMWORD_T  bnblk = memgetbufnblk(buf);
    MEMWORD_T  bnfree = memgetbufnfree(buf);
    MEMWORD_T  btype = memgetbuftype(buf);
    MEMWORD_T  bslot = memgetbufslot(buf);
    MEMUWORD_T bsz = membufblksize(buf, type, slot);
    MEMUWORD_T bufsz;
    long       fail = 0;

    if ((flg) && !bflg) {
        fprintf(stderr, "HEAP-bit not set\n");
        fail++;
    }
    if ((nblk) && bnblk != nblk) {
        fprintf(stderr, "WRONG block count: %lx (%lx)\n", bnblk, nblk);
        fail++;
    }
    if (bnfree > bnblk) {
        fprintf(stderr, "nfree > nblk\n");
        fail++;
    }
    if ((bnfree) && !buf->bkt) {
        fprintf(stderr, "NOT in any bucket (%lx free blocks)\n", bnblk);
        fail++;
    }
    if (type != btype) {
        fprintf(stderr, "WRONG type: %lx (%s)\n", btype, bufnames[type]);
        fail++;
    }
    if (fail) {
        memprintbuf(buf, func);

        abort();
    }

    return 0;
}

#endif

