#include <stddef.h>
#if (MEMDEBUG)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/mem.h>
#include <zero/priolk.h>

typedef struct membuf * memallocbuffunc(struct mem *mem, long slot);
typedef void * meminitbuffunc(struct mem *mem, struct membuf *buf);

static volatile THREADLOCAL struct memarn *tls_arn ALIGNED(PAGESIZE);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
static THREADLOCAL struct priolkdata       tls_priolkdata;
#endif
static THREADLOCAL pthread_once_t          tls_once;
static THREADLOCAL pthread_key_t           tls_key;
static THREADLOCAL long                    tls_flg;
struct mem                                 g_mem;

static void
memfreetls(void *arg)
{
    ;
}

static unsigned long
memgetprioval(void)
{
    unsigned long val = g_mem.prioval;

    val++;
    val &= sizeof(long) * CHAR_BIT - 1;
    g_mem.prioval = val;

    return;
}

static struct memarn *
meminitarn(void)
{
    struct memarn *arn = NULL;
    unsigned long  val;
    long           slot;

    arn = mapanon(0, MEMARNSIZE);
    if (arn) {
        val = memgetprioval();
        pthread_key_create(&tls_key, memfreetls);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
        priolkinit(&tls_priolkdata, val);
#endif
        for (slot = 0 ; slot < PTRBITS ; slot++) {
            arn->smallbin[slot].slot = slot;
            arn->pagebin[slot].slot = slot;
        }
        tls_arn = arn;
    }

    return arn;
}

static void
meminit(void)
{
    void *heap;
    long  ofs;
    void *ptr;

#if (MEMDEBUG)
    fprintf(stderr, "MEMALIGNSHIFT: %ld\n", MEMALIGNSHIFT);
#endif
    memgetlk(&g_mem.initlk);
    if (g_mem.flg & MEMINITBIT) {
        memrellk(&g_mem.initlk);

        return;
    }
    ptr = mapanon(0, MEMLVL1ITEMS * sizeof(struct memtab));
    if (ptr == MAP_FAILED) {

        abort();
    }
    g_mem.tab = ptr;
    memgetlk(&g_mem.heaplk);
    heap = growheap(0);
    ofs = (1UL << PAGESIZELOG2) - ((long)heap & (PAGESIZE - 1));
    if (ofs != PAGESIZE) {
        growheap(ofs);
    }
    memrellk(&g_mem.heaplk);
    g_mem.flg |= MEMINITBIT;
    memrellk(&g_mem.initlk);

    return;
}

static struct membuf *
memallocsmallbuf(struct mem *mem, long slot)
{
    MEMPTR_T       adr = SBRK_FAILED;
    MEMWORD_T      bufsz = memsmallbufsize(slot);
    MEMUWORD_T     info = 0;
    struct membuf *buf;

    if (!(mem->flg & MEMNOHEAPBIT)) {
        /* try to allocate from heap (sbrk()) */
        memgetlk(&mem->heaplk);
        adr = growheap(bufsz);
        if (adr == SBRK_FAILED) {
            memrellk(&mem->heaplk);
        } else {
            info = MEMHEAPBIT;
        }
    }
    if (adr == SBRK_FAILED) {
        /* sbrk() failed, let's try mmap() */
        memrellk(&mem->heaplk);
        adr = mapanon(0, bufsz);
        if (adr == MAP_FAILED) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif

            return NULL;
        }
    }
    buf = (struct membuf *)adr;
    buf->info = info;             // possible MEMHEAPBIT
    memsetbufslot(buf, slot);
    buf->size = bufsz;
    buf->bkt = &tls_arn->smallbin[slot];
    buf->ptrtab = (MEMPTR_T)buf + membufhdrsize();

    return buf;
}

static void *
meminitsmallbuf(struct mem *mem, struct membuf *buf)
{
    MEMWORD_T  nblk = MEMBUFFREEMAPWORDS * CHAR_BIT * sizeof(MEMWORD_T);
    long       slot = membufslot(buf);
    MEMPTR_T   adr = (MEMPTR_T)buf;
    MEMWORD_T  blksz = MEMWORD(1) << (slot);
    MEMUWORD_T info = buf->info;
    MEMPTR_T   ptr = adr + membufblkofs();
    struct membuf *bptr;

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;
    if (info & MEMHEAPBIT) {
        /* link block from sbrk() to global heap (put it on top) */
        bptr = mem->heap;
        buf->heap = bptr;
        memrellk(&mem->heaplk);
    }
    VALGRINDMKPOOL(ptr, 0, 0);

    return ptr;
}

static struct membuf *
memallocpagebuf(struct mem *mem, long slot, MEMWORD_T nblk)
{
    MEMWORD_T      mapsz = mempagebufsize(slot, nblk);
    MEMPTR_T       adr;
    struct membuf *buf;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    buf = (struct membuf *)adr;
    memsetbufslot(buf, slot);
    buf->size = mapsz;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    buf->bkt = &tls_arn->pagebin[slot];
    buf->ptrtab = (MEMPTR_T)buf + membufhdrsize();

    return buf;
}

static void *
meminitpagebuf(struct mem *mem, struct membuf *buf, MEMUWORD_T nblk)
{
    long      slot = membufslot(buf);
    MEMPTR_T  adr = (MEMPTR_T)buf;
    MEMWORD_T blksz = slot * PAGESIZE;
    MEMPTR_T  ptr = adr + membufblkofs();

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;
    VALGRINDMKPOOL(ptr, 0, 0);

    return ptr;
}

static struct membuf *
memallocbigbuf(struct mem *mem, long slot, MEMUWORD_T nblk)
{
    MEMWORD_T      mapsz = membigbufsize(slot, nblk);
    MEMPTR_T       adr;
    struct membuf *buf;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    buf = (struct membuf *)adr;
    memsetbufslot(buf, slot);
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    buf->size = mapsz;
    buf->bkt = &g_mem.smallbin[slot];
    buf->ptrtab = (MEMPTR_T)buf + membufhdrsize();

    return buf;
}

static void *
meminitbigbuf(struct mem *mem, struct membuf *buf, MEMUWORD_T nblk)
{
    long       slot = membufslot(buf);
    MEMPTR_T   adr = (MEMPTR_T)buf;
    MEMUWORD_T blksz = MEMWORD(1) << (slot);
    MEMPTR_T   ptr = adr + membufblkofs();

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;
    VALGRINDMKPOOL(ptr, 0, 0);

    return ptr;
}

struct membuf *
membufgetblk(struct membuf *head, MEMPTR_T *retptr, struct membkt *bkt)
{
    MEMWORD_T      ndx = (head) ? membufgetfree(head) : -1;
    struct membuf *prev;

    *retptr = NULL;
    while ((head) && ndx < 0) {
        /* remove fully-allocated bufs from the front of list */
        prev = head;
        if (head->next) {
            head->next->prev = NULL;
        }
        head = head->next;
        prev->bkt = NULL;
        prev->prev = NULL;
        prev->next = NULL;
        if (head) {
            ndx = membufgetfree(head);
        }
    }
    if (head) {
        if (head->next) {
            head->next->prev = NULL;
        }
        head->prev = NULL;
        head->next = NULL;
        head->bkt = bkt;
        *retptr = membufblkadr(head, ndx);
    }
#if (MEMDEBUG)
    fprintf(stderr, "%p\n", *retptr);
#endif

    return head;
}

struct membuf *
membufgetpages(struct membuf *head, MEMPTR_T *retptr, struct membkt *bkt)
{
    MEMWORD_T      ndx = membufgetfree(head);
    struct membuf *prev;

    *retptr = NULL;
    while ((head) && ndx < 0) {
        /* remove fully-allocated bufs from the front of list */
        prev = head;
        if (head->next) {
            head->next->prev = NULL;
        }
        head = head->next;
        if (head) {
            ndx = membufgetfree(head);
        }
        prev->prev = NULL;
        prev->next = NULL;
    }
    if (head) {
        if (head->next) {
            head->next->prev = NULL;
        }
        head->prev = NULL;
        head->next = NULL;
        head->bkt = bkt;
        *retptr = membufpageadr(head, ndx);
    }

    return head;
}

/* find a buf address; type encoded in the low 2 bits */
void *
memputbuf(void *ptr, struct membuf *buf)
{
    struct memitem *itab;
    struct memitem *item;
    long            k1;
    long            k2;
    long            k3;
#if (ADRBITS > 52)
    long            k4;
    void           *pstk[2] = { NULL };
#else
    void           *pstk[1] = { NULL };
#endif
    
#if (ADRBITS > 52)
    memgetkeybits(ptr, k1, k2, k3, k4);
#else
    memgetkeybits(ptr, k1, k2, k3);
#endif
    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memitem));
        if (itab == MAP_FAILED) {
            
            return NULL;
        }
        pstk[0] = itab;
        g_mem.tab[k1].tab = itab;
    }
    item = &itab[k2];
    itab = item->tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memitem));
        if (itab == MAP_FAILED) {
            unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memitem));
            
            return NULL;
        }
#if (ADRBITS > 52)
        pstk[1] = itab;
#endif
        item->tab = itab;
    }
#if (ADRBITS <= 52)
    ((MEMADR_T *)itab)[k3] = (MEMADR_T)buf;
#else
    item = &itab[k3];
    itab = item->tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(MEMADR_T));
        if (itab == MAP_FAILED) {
            unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memitem));
            unmapanon(pstk[1], MEMLVLITEMS * sizeof(struct memitem));
             
            return NULL;
        }
        item->tab = itab;
    }
    ((MEMADR_T *)itab)[k4] = (MEMADR_T)buf;
#endif
    memrellk(&g_mem.tab[k1].lk);
    
    return ptr;
}

struct membuf *
memfindbuf(void *ptr)
{
    MEMADR_T        ret = 0;
    struct memitem *itab;
    struct memitem *item;
    long            k1;
    long            k2;
    long            k3;
#if (ADRBITS > 52)
    long            k4;
#endif

#if (ADRBITS > 52)
    memgetkeybits(ptr, k1, k2, k3, k4);
#else
    memgetkeybits(ptr, k1, k2, k3);
#endif
    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (itab) {
        item = &itab[k2];
        itab = item->tab;
#if (ADRBITS <= 52)
        if (itab) {
            ret = ((MEMADR_T *)itab)[k3];
        }
#else
        if (itab) {
            item = &itab[k3];
            itab = item->tab;
            if (itab) {
                ret = ((MEMADR_T *)itab)[k4];
            }
        }
#endif
    }
    memrellk(&g_mem.tab[k1].lk);

    return (struct membuf *)ret;
}

static MEMADR_T
memopenbuf(struct membkt *bkt)
{
    MEMADR_T upval;

    memlkbit((volatile m_atomic_t *)&bkt->list);
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;

    return upval;
}

void
memputptr(struct membuf *buf, void *ptr, size_t align, long type)
{
    MEMPTR_T adr;
    
    adr = ptr;
    if ((MEMADR_T)ptr & (align - 1)) {
        ptr = memalignptr(adr, align);
    }
    if (type != MEMPAGEBLK) {
        membufsetptr(buf, ptr, adr);
    } else {
        membufsetpage(buf, ptr, adr);
    }
    memputbuf(ptr, buf);

    return;
}

MEMPTR_T
memgetblk(long slot, long type, size_t align)
{
    struct memarn *arn;
    MEMPTR_T       ptr = NULL;
    MEMPTR_T       adr = NULL;
    struct membkt *bkt;
    struct membuf *buf;
    MEMUWORD_T     info = 0;
    MEMUWORD_T     nblk = membufnblk(slot, type);
    struct membuf *bptr;
    MEMADR_T       upval;
    MEMADR_T       bufval;

    if (!tls_arn && !meminitarn()) {

        exit(1);
    }
    if (!(g_mem.flg & MEMINITBIT)) {
        meminit();
    }
    arn = tls_arn;
    if (!type) {
        bkt = &arn->smallbin[slot];
        upval = memopenbuf(bkt);
        buf = (struct membuf *)upval;
        if (upval) {
            /* TODO */
            buf = membufgetblk(buf, &ptr, bkt);
            if (!buf) {
                buf = memallocsmallbuf(&g_mem, slot);
                if (buf) {
                    ptr = meminitsmallbuf(&g_mem, buf);
                    if (ptr) {
                        memputptr(buf, ptr, align, type);
                    }
                    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                } else {
                    m_clrbit((m_atomic_t *)&bkt->list, MEMLKBITID);
                }
            } else {
                bufval = (MEMADR_T)buf;
                m_syncwrite((m_atomic_t *)&bkt->list, bufval);
                buf = (struct membuf *)bufval;
                if (bufval == upval) {
                    memputptr(buf, ptr, align, type);
                    
                    return ptr;
                }
                buf = (struct membuf *)bufval;
            }
        } else {
            /* TODO: try global buffer */
            buf = memallocsmallbuf(&g_mem, slot);
            if (buf) {
                ptr = meminitsmallbuf(&g_mem, buf);
                if (ptr) {
                    memputptr(buf, ptr, align, type);
                }
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
            } else {
                m_clrbit((m_atomic_t *)&bkt->list, MEMLKBITID);
            }
        }
    } else if (type == 1) {
        bkt = &arn->pagebin[slot];
        upval = memopenbuf(bkt);
        buf = (struct membuf *)upval;
        if (upval) {
            buf = membufgetpages(buf, &ptr, bkt);
            if (!buf) {
                buf = memallocpagebuf(&g_mem, slot, nblk);
                if (buf) {
                    ptr = meminitpagebuf(&g_mem, buf, nblk);
                    if (ptr) {
                        memputptr(buf, ptr, align, type);
                    }
                    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                } else {
                    m_clrbit((m_atomic_t *)&bkt->list, MEMLKBITID);
                }
            } else {
                bufval = (MEMADR_T)buf;
                m_syncwrite((m_atomic_t *)&bkt->list, bufval);
                if (bufval == upval) {
                    memputptr(buf, ptr, align, type);
                    
                    return ptr;
                }
                buf = (struct membuf *)bufval;
            }
        } else {
            /* TODO: try global buffer */
            buf = memallocpagebuf(&g_mem, slot, nblk);
            if (buf) {
                ptr = meminitpagebuf(&g_mem, buf, nblk);
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
            } else {
                m_clrbit((m_atomic_t *)&bkt->list, MEMLKBITID);
            }
        }
    } else {
        bkt = &g_mem.bigbin[slot];
        upval = memopenbuf(bkt);
        buf = (struct membuf *)upval;
        if (upval) {
            buf = membufgetpages(buf, &ptr, bkt);
            if (!buf) {
                buf = memallocbigbuf(&g_mem, slot, nblk);
                if (buf) {
                    ptr = meminitbigbuf(&g_mem, buf, nblk);
                    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                } else {
                    m_clrbit((m_atomic_t *)&bkt->list, MEMLKBITID);
                }
            } else {
                bufval = (MEMADR_T)buf;
                m_syncwrite((m_atomic_t *)&bkt->list, bufval);
                if (bufval == upval) {
                    memputptr(buf, ptr, align, type);
                    
                    return ptr;
                }
                buf = (struct membuf *)bufval;
            }
        } else {
            buf = memallocbigbuf(&g_mem, slot, nblk);
            if (buf) {
                ptr = meminitbigbuf(&g_mem, buf, nblk);
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
            } else {
                m_clrbit((m_atomic_t *)&bkt->list, MEMLKBITID);
            }
        }
    }
    if (buf) {
        /* link buf to bucket */
        upval = (MEMADR_T)bkt->list;
        upval &= ~MEMLKBIT;
        bptr = (struct membuf *)upval;
        if (bptr) {
            bptr->prev = buf;
        }
        buf->prev = NULL;
        buf->next = bptr;
        buf->bkt = bkt;
//        buf->ptrtab = (MEMPTR_T)buf + membufhdrsize();
        if (info & MEMHEAPBIT) {
            /* this unlocks the global heap (low-bit becomes zero) */
            m_syncwrite(&g_mem.heap, buf);
        }
        memputptr(buf, ptr, align, type);
        /* this unlocks the arena bucket (low-bit becomes zero) */
        m_syncwrite((m_atomic_t *)&bkt->list, buf);
    } else {
        memrelbit((m_atomic_t *)&bkt->list);
    }
    if (ptr) {
        memputbuf(ptr, buf);
    }

    return ptr;
}

void
memputblk(void *ptr, struct membuf *buf)
{
    struct membkt *bkt = buf->bkt;
    MEMWORD_T      ins = 0;
    MEMWORD_T      slot;
    MEMWORD_T      type;
    MEMADR_T       upval;
    MEMPTR_T      *adr;
    MEMWORD_T      id;

    if (!ptr) {

        return;
    }
    if (!tls_arn && !meminitarn()) {

        exit(1);
    }
    if (bkt) {
        memlkbit((m_atomic_t *)&bkt->list);
    }
    type = memgetbuftype(buf);
    slot = memgetbufslot(buf);
    if (!bkt) {
        ins = 1;
        if (type == MEMSMALLBLK) {
            bkt = &tls_arn->smallbin[slot];
        } else if (type == MEMPAGEBLK) {
            bkt = &tls_arn->pagebin[slot];
        } else {
            bkt = &g_mem.bigbin[slot];
        }
        memlkbit((m_atomic_t *)&bkt->list);
    }
    if (type != MEMPAGEBLK) {
        adr = membufgetptr(buf, ptr);
    } else {
        adr = membufgetpage(buf, ptr);
    }
    if (type == MEMSMALLBLK || type == MEMBIGBLK) {
        id = membufblkid(buf, ptr);
    } else {
        id = membufpageid(buf, ptr);
    }
    clrbit(buf->freemap, id);
    if (ins) {
        upval = bkt->list;
        upval &= ~MEMLKBIT;
        buf->bkt = bkt;
        buf->next = (struct membuf *)upval;
        if (upval) {
            buf->next->prev = buf;
        }
        m_syncwrite((m_atomic_t)&bkt->list, (m_atomic_t)buf);
    } else {
        memrelbit((m_atomic_t *)&bkt->list);
    }

    return;
}

