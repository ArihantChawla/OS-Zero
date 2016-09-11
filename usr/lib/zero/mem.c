#include <stddef.h>
#include <stdio.h>
#if (MEMDEBUG)
#include <stdio.h>
#include <assert.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/mem.h>
#if (MEM_LK_TYPE == MEM_LK_PRIO)
#include <zero/priolk.h>
#endif

#if (MEM_LK_TYPE == MEM_LK_PRIO)
THREADLOCAL struct priolkdata       tls_priolkdata;
#endif
THREADLOCAL volatile struct memarn *tls_arn;
THREADLOCAL pthread_once_t          tls_once;
THREADLOCAL pthread_key_t           tls_key;
THREADLOCAL long                    tls_flg;
struct mem                          g_mem;

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

    return val;
}

struct memarn *
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

void
meminit(void)
{
    void *heap;
    long  ofs;
    void *ptr;

#if (MEMDEBUG)
    fprintf(stderr, "MEMALIGNSHIFT: %ld\n", MEMALIGNSHIFT);
    fprintf(stderr, "LVL1: %ld (%ld)\n", MEMLVL1ITEMS, MEMLVLITEMS);
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
memallocsmallbuf(long slot)
{
    MEMPTR_T       adr = SBRK_FAILED;
    MEMWORD_T      bufsz = memsmallbufsize(slot);
    MEMUWORD_T     info = 0;
    struct membuf *buf;

    if (!(g_mem.flg & MEMNOHEAPBIT)) {
        /* try to allocate from heap (sbrk()) */
        memgetlk(&g_mem.heaplk);
        adr = growheap(bufsz);
        if (adr != SBRK_FAILED) {
            info = MEMHEAPBIT;
        }
        memrellk(&g_mem.heaplk);
    }
    if (adr == SBRK_FAILED) {
        /* sbrk() failed, let's try mmap() */
        memrellk(&g_mem.heaplk);
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
    memsetbufnblk(buf, MEMBUFBLKS);
    buf->size = bufsz;
    buf->bkt = &tls_arn->smallbin[slot];
    buf->ptrtab = (MEMPTR_T *)((MEMPTR_T)buf + membufhdrsize());

    return buf;
}

static void *
meminitsmallbuf(struct membuf *buf)
{
    MEMWORD_T  nblk = MEMBUFBLKS;
    MEMPTR_T   adr = (MEMPTR_T)buf;
    MEMUWORD_T info = buf->info;
    MEMPTR_T   ptr = adr + membufblkofs();
    struct membuf *bptr;

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;
    nblk--;
    if (info & MEMHEAPBIT) {
        /* link block from sbrk() to global heap (put it on top) */
        bptr = g_mem.heap;
        buf->heap = bptr;
        memrellk(&g_mem.heaplk);
    }
    memsetbufnfree(buf, nblk);
    VALGRINDMKPOOL(ptr, 0, 0);

    return ptr;
}

static struct membuf *
memallocpagebuf(long slot, MEMWORD_T nblk)
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
    buf->ptrtab = (MEMPTR_T *)((MEMPTR_T)buf + membufhdrsize());

    return buf;
}

static void *
meminitpagebuf(struct membuf *buf, MEMUWORD_T nblk)
{
    MEMPTR_T  adr = (MEMPTR_T)buf;
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
memallocbigbuf(long slot, MEMUWORD_T nblk)
{
    MEMWORD_T      mapsz = membigbufsize(slot, nblk);
    MEMPTR_T       adr;
    struct membuf *buf;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    buf = (struct membuf *)adr;
    memsetbufslot(buf, slot);
    nblk--;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    memsetbufnfree(buf, nblk);
    buf->size = mapsz;
    buf->bkt = &g_mem.smallbin[slot];
    buf->ptrtab = (MEMPTR_T *)((MEMPTR_T)buf + membufhdrsize());

    return buf;
}

static void *
meminitbigbuf(struct membuf *buf, MEMUWORD_T nblk)
{
    MEMPTR_T adr = (MEMPTR_T)buf;
    MEMPTR_T ptr = adr + membufblkofs();

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    nblk--;
    buf->base = ptr;
    memsetbufnfree(buf, nblk);
    VALGRINDMKPOOL(ptr, 0, 0);

    return ptr;
}

void *
memgetbufblk(struct membuf *head, struct membkt *bkt, MEMUWORD_T *retinfo)
{
    void          *ptr = NULL;
    MEMWORD_T      nfree = (head) ? memgetbufnfree(head) : 0;
    MEMWORD_T      ndx;
    struct membuf *prev;

    while ((head) && !nfree) {
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
            nfree = memgetbufnfree(head);
        }
    }
    if (head) {
        ndx = membufgetfree(head);
        nfree--;
        ptr = membufblkadr(head, ndx);
        memsetbufnfree(head, nfree);
        if (!nfree) {
            bkt = head->bkt;
            if (head->next) {
                head->next->prev = NULL;
            }
            head->bkt = NULL;
            head->prev = NULL;
            head->next = NULL;
            head = head->next;
            bkt->nbuf--;
        }
        if (retinfo) {
            *retinfo = ndx;
        }
    }
    m_syncwrite((m_atomic_t *)&bkt->list, head);

    return ptr;
}

void *
memputbuf(void *ptr, struct membuf *buf, MEMUWORD_T info)
{
    MEMADR_T        val = (MEMADR_T)buf | info;
    struct memtab  *itab;
    struct memtab  *tab;
    struct memitem *item;
    m_atomic_t     *lk1;
    m_atomic_t     *lk2;
    long             k1;
    long             k2;
    long             k3;
#if (ADRBITS >= 48)
    long             k4;
    void            *pstk[2] = { NULL };
#else
    void            *pstk[1] = { NULL };
#endif
    
#if (ADRBITS >= 48)
    memgetkeybits(ptr, k1, k2, k3, k4);
#else
    memgetkeybits(ptr, k1, k2, k3);
#endif
    lk1 = &g_mem.tab[k1].tab;
    memlkbit(lk1);
    itab = (struct memtab *)((MEMADR_T)g_mem.tab[k1].tab & ~MEMLKBIT);
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memtab));
        if (itab == MAP_FAILED) {
            memrelbit(lk1);
            
            return NULL;
        }
        pstk[0] = itab;
        g_mem.tab[k1].tab = (struct memtab *)((MEMADR_T)itab | MEMLKBIT);
    }
    tab = &itab[k2];
    lk2 = &tab->tab;
    if (tab->tab) {
        memlkbit(lk2);
        memrelbit(lk1);
        itab = (struct memtab *)((MEMADR_T)tab->tab & ~MEMLKBIT);
    } else {
#if (ADRBITS >= 48)
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memtab));
#else
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memitem));
#endif
        if (itab == MAP_FAILED) {
            unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memtab));
            memrelbit(lk1);
            
            return NULL;
        }
#if (ADRBITS >= 48)
        pstk[1] = itab;
#endif
        lk2 = &tab->tab;
        tab->tab = (struct memtab *)((MEMADR_T)itab | MEMLKBIT);
        memrelbit(lk1);
    }
#if (ADRBITS < 48)
    item = (struct memitem *)&itab[k3];
    item->nref++;
    item->val = val;
    memrelbit(lk2);
#else
    tab = &itab[k3];
    lk1 = &tab->tab;
    if (tab->tab) {
        memlkbit(lk1);
        memrelbit(lk2);
        itab = (struct memtab *)((MEMADR_T)tab->tab & ~MEMLKBIT);
    } else {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memitem));
        if (itab == MAP_FAILED) {
            unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memtab));
            unmapanon(pstk[1], MEMLVLITEMS * sizeof(struct memtab));
            
            return NULL;
        }
        tab->tab = (struct memtab *)((MEMADR_T)itab | MEMLKBIT);
        memrelbit(lk2);
    }
    item = (struct memitem *)&itab[k4];
    item->nref++;
    item->val = val;
    memrelbit(lk1);
#endif
    
    return ptr;
}

MEMADR_T
memfindbuf(void *ptr, long rel)
{
    MEMADR_T        ret = 0;
    struct membuf  *buf = NULL;
    struct memtab  *itab;
    struct memtab  *tab;
    struct memitem *item;
    m_atomic_t     *lk1;
    m_atomic_t     *lk2;
    long            k1;
    long            k2;
    long            k3;
#if (ADRBITS >= 48)
    long            k4;
#endif

#if (ADRBITS >= 48)
    memgetkeybits(ptr, k1, k2, k3, k4);
#else
    memgetkeybits(ptr, k1, k2, k3);
#endif
//    memgetlk(&g_mem.tab[k1].lk);
    lk1 = &g_mem.tab[k1].tab;
    memlkbit(lk1);
    itab = (struct memtab *)((MEMADR_T)g_mem.tab[k1].tab & ~MEMLKBIT);
    if (itab) {
        tab = &itab[k2];
        lk2 = &tab->tab;
        memlkbit(lk2);
        memrelbit(lk1);
        itab = (struct memtab *)((MEMADR_T)tab->tab & ~MEMLKBIT);
#if (ADRBITS < 48)
        if (itab) {
            item = (struct membufitam *)&itab[k3];
            ret = item->val;
            if (rel) {
                if (!--item->nref) {
                    item->val = 0;
                }
            }
        }
        memrelbit(lk2);
#else
        if (itab) {
            tab = &itab[k3];
            lk1 = &tab->tab;
            memlkbit(lk1);
            memrelbit(lk2);
            itab = (struct memtab *)((MEMADR_T)tab->tab & ~MEMLKBIT);
            if (itab) {
                item = (struct memitem *)&itab[k4];
                ret = item->val;
                if (rel) {
                    if (!--item->nref) {
                        item->val = 0;
                    }
                }
            }
            memrelbit(lk1);
        } else {
            memrelbit(lk2);
        }
#endif
    } else {
        memrelbit(lk1);
    }
//    memrellk(&g_mem.tab[k1].lk);

    return buf;
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

/* for pagebin, val is the allocation index */
void
memputptr(struct membuf *buf, void *ptr, size_t align, long info)
{
    MEMUWORD_T ndx = info & MEMPAGENDXMASK;
    MEMPTR_T   adr;
    
    adr = ptr;
    if ((MEMADR_T)ptr & (align - 1)) {
        ptr = memalignptr(adr, align);
    }
    if (!ndx) {
        membufsetptr(buf, ptr, adr);
    } else {
        membufsetpage(buf, ndx, adr);
    }
    memputbuf(ptr, buf, info);

    return;
}

MEMPTR_T
memgetblk(long slot, long type, size_t align)
{
    struct memarn *arn;
    MEMPTR_T       ptr = NULL;
    struct membkt *bkt;
    struct membuf *buf;
    MEMUWORD_T     info = 0;
    MEMUWORD_T     flg = 0;
    MEMUWORD_T     nblk = membufnblk(slot, type);
    struct membuf *bptr;
    MEMADR_T       upval = NULL;
    MEMADR_T       bufval = NULL;

    arn = tls_arn;
    if (type == MEMSMALLBLK) {
        bkt = &g_mem.smallbin[slot];
        upval = memopenbuf(bkt);
        if (upval) {
            buf = (struct membuf *)upval;
        } else {
            memrelbit((m_atomic_t *)&bkt->list);
            bkt = &arn->smallbin[slot];
            upval = memopenbuf(bkt);
            buf = (struct membuf *)upval;
            if (upval) {
                /* TODO */
                ptr = memgetbufblk(buf, bkt, NULL);
                if (!ptr) {
                     memrelbit((m_atomic_t *)&bkt->list);
                     buf = memallocsmallbuf(slot);
                     if (buf) {
                         ptr = meminitsmallbuf(buf);
                         if (ptr) {
                             memputptr(buf, ptr, align, 0);
                         }
                         memlkbit((m_atomic_t *)&bkt->list);
                         upval = (MEMADR_T)bkt->list;
                         upval &= ~MEMLKBIT;
                         buf->next = (struct membuf *)upval;
                         if (buf->next) {
                             buf->next->prev = buf;
                         }
                         m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                     }
                } else {
                    memrelbit((m_atomic_t *)&bkt->list);
                }
            } else {
                /* TODO: try global buffer */
                buf = memallocsmallbuf(slot);
                if (buf) {
                    ptr = meminitsmallbuf(buf);
                    if (ptr) {
                        memputptr(buf, ptr, align, 0);
                    }
                    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                } else {
                    memrelbit((m_atomic_t *)&bkt->list);
                }
            }
        }
    } else if (type == MEMPAGEBLK) {
        bkt = &g_mem.pagebin[slot];
        upval = memopenbuf(bkt);
        if (upval) {
            buf = (struct membuf *)upval;
        } else {
            memrelbit((m_atomic_t *)&bkt->list);
            bkt = &arn->pagebin[slot];
            upval = memopenbuf(bkt);
            buf = (struct membuf *)upval;
            if (upval) {
                ptr = memgetbufblk(buf, bkt, &info);
                if (!ptr) {
                    memrelbit((m_atomic_t *)&bkt->list);
                    buf = memallocpagebuf(slot, nblk);
                    if (buf) {
                        ptr = meminitpagebuf(buf, nblk);
                        if (ptr) {
                            memputptr(buf, ptr, align, 0 | MEMPAGEBIT);
                        }
                        memlkbit((m_atomic_t *)&bkt->list);
                        upval = (MEMADR_T)bkt->list;
                        upval &= ~MEMLKBIT;
                        buf->next = (struct membuf *)upval;
                        if (buf->next) {
                            buf->next->prev = buf;
                        }
                        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                    }
                } else {
                    flg |= MEMPAGEBIT;
                    memrelbit((m_atomic_t *)&bkt->list);
                }
            } else {
                /* TODO: try global buffer */
                buf = memallocpagebuf(slot, nblk);
                if (buf) {
                    ptr = meminitpagebuf(buf, nblk);
                    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                } else {
                    memrelbit((m_atomic_t *)&bkt->list);
                }
            }
        }
    } else {
        bkt = &g_mem.bigbin[slot];
        upval = memopenbuf(bkt);
        if (upval) {
            buf = (struct membuf *)upval;
        } else {
            memrelbit((m_atomic_t *)&bkt->list);
            buf = memallocbigbuf(slot, nblk);
            if (buf) {
                ptr = meminitbigbuf(buf, nblk);
                if (ptr) {
                    memputptr(buf, ptr, align, 0);
                }
                memlkbit((m_atomic_t *)&bkt->list);
                upval = (MEMADR_T)bkt->list;
                upval &= ~MEMLKBIT;
                buf->next = (struct membuf *)upval;
                if (buf->next) {
                    buf->next->prev = buf;
                }
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
            }
        }
    }
    if ((buf) && (ptr)) {
        if (flg & MEMHEAPBIT) {
            memlkbit(&g_mem.heap);
            upval = (MEMADR_T)g_mem.heap;
            upval &= ~MEMLKBIT;
            buf->heap = (struct membuf *)upval;
            /* this unlocks the global heap (low-bit becomes zero) */
            m_syncwrite(&g_mem.heap, buf);
        }
        if (bkt == &g_mem.smallbin[slot]) {
            bkt = &arn->smallbin[slot];
            memlkbit(&bkt->list);
        } else if (bkt == &g_mem.pagebin[slot]) {
            bkt = &arn->pagebin[slot];
            memlkbit(&bkt->list);
        }
        memputptr(buf, ptr, align, info);
        /* this unlocks the arena bucket (low-bit becomes zero) */
        m_syncwrite((m_atomic_t *)&bkt->list, buf);
    } else {
        memrelbit((m_atomic_t *)&bkt->list);
    }
    if (ptr) {
        memputbuf(ptr, buf, info);
    }

    return ptr;
}

#if 0
void
memrelblk(void *ptr, struct membuf *buf)
{
    struct membkt *bkt = buf->bkt;
    MEMWORD_T      nblk = memgetbufnblk(buf);
    MEMWORD_T      nfree = memgetbufnfree(buf);
    MEMWORD_T      ins = 0;
    MEMPTR_T       adr;
    MEMWORD_T      slot;
    MEMWORD_T      type;
    MEMADR_T       upval;
    MEMWORD_T      id;

    if (!ptr) {

        return;
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
#if 0
    if (type != MEMPAGEBLK) {
        adr = membufgetptr(buf, ptr);
        membufsetptr(buf, ptr, NULL);
    } else {
        adr = membufgetpage(buf, ptr);
        membufsetpage(buf, ptr, NULL);
    }
#endif
    adr = membufgetptr(buf, ptr);
    membufsetptr(buf, ptr, NULL);
    nfree++;
    if (type == MEMSMALLBLK || type == MEMBIGBLK) {
        id = membufblkid(buf, adr);
    } else {
        id = membufpageid(buf, adr);
    }
    setbit(buf->freemap, id);
    memsetbufnfree(buf, nfree);
    if (ins) {
        if (!(buf->info & MEMHEAPBIT)
            && ((type <= MEMPAGEBLK && bkt->nbuf >= membufnarn(slot, type))
                || (type == MEMBIGBLK
                    && bkt->nbuf >= membufnglob(slot, type)))) {
            unmapanon(buf, buf->size);
            memrelbit((m_atomic_t *)&bkt->list);
        } else {
            upval = (MEMADR_T)bkt->list;
            upval &= ~MEMLKBIT;
            buf->bkt = bkt;
            buf->prev = NULL;
            buf->next = (struct membuf *)upval;
            if (upval) {
                buf->next->prev = buf;
            }
            bkt->nbuf++;
            m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
        }
    } else {
        memrelbit((m_atomic_t *)&bkt->list);
    }

    return;
}
#endif

void
memrelblk(void *ptr, struct membuf *buf, MEMUWORD_T info)
{
    struct membkt *bkt = buf->bkt;
    MEMUWORD_T     ndx = info & MEMPAGENDXMASK;
    MEMWORD_T      nblk = memgetbufnblk(buf);
    MEMWORD_T      nfree = memgetbufnfree(buf);
    MEMWORD_T      ins = 0;
    MEMPTR_T       adr;
    MEMWORD_T      slot;
    MEMWORD_T      type;
    MEMADR_T       upval;
    MEMWORD_T      id;

    if (bkt) {
        memlkbit((m_atomic_t *)&bkt->list);
    }
    type = memgetbuftype(buf);
    slot = memgetbufslot(buf);
    if (!bkt) {
//        ins = 1;
        if (type == MEMSMALLBLK) {
            bkt = &tls_arn->smallbin[slot];
        } else if (type == MEMPAGEBLK) {
            bkt = &tls_arn->pagebin[slot];
        } else {
            bkt = &g_mem.bigbin[slot];
        }
        memlkbit((m_atomic_t *)&bkt->list);
    }
    nfree++;
    if (!(info & MEMPAGEBIT)) {
        adr = membufgetptr(buf, ptr);
        membufsetptr(buf, ptr, NULL);
        id = membufblkid(buf, adr);
    } else {
        adr = membufpageadr(buf, ndx);
        membufsetpage(buf, ndx, NULL);
        id = ndx;
    }
    setbit(buf->freemap, id);
    memsetbufnfree(buf, nfree);
    if (nfree == 1 && nblk > 1) {
        upval = (MEMADR_T)bkt->list;
        upval &= ~MEMLKBIT;
        buf->next = (struct membuf *)upval;
        if (buf->next) {
            buf->next->prev = buf;
        }
        m_syncwrite((m_atomic_t *)&bkt->list, buf);
    }
    if (bkt) {
        memrelbit((m_atomic_t *)&bkt->list);
    }

    return;
}

