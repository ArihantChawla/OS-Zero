#include <stddef.h>
#include <stdio.h>
#if (MEMDEBUG)
#include <stdio.h>
//#include <crash.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/mem.h>
#include <zero/hash.h>
#if (MEM_LK_TYPE == MEM_LK_PRIO)
#include <zero/priolk.h>
#endif

THREADLOCAL volatile struct memtls *g_memtls;
struct mem                          g_mem;

static void
memfreetls(void *arg)
{
    long slot;

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

struct memtls *
meminittls(void)
{
    struct memtls *tls;
    unsigned long  val;
//    long           slot;

    tls = mapanon(0, MEMTLSSIZE);
    if (tls != MAP_FAILED) {
        val = memgetprioval();
        pthread_key_create(&tls->key, memfreetls);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
        priolkinit(&tls->priolkdata, val);
#endif
#if 0
        for (slot = 0 ; slot < PTRBITS ; slot++) {
            tls->smallbin[slot].slot = slot;
        }
        for (slot = 0 ; slot < MEMPAGEBINS ; slot++) {
            tls->pagebin[slot].slot = slot;
        }
#endif
        g_memtls = tls;
    }

    return tls;
}

static void
memprefork(void)
{
    long bin;

    memgetlk(&g_mem.initlk);
    memgetlk(&g_mem.heaplk);
    for (bin = 0 ; bin < PTRBITS ; bin++) {
        memlkbit(&g_mem.smallbin[bin].list);
        memlkbit(&g_mem.bigbin[bin].list);
    }
    for (bin = 0 ; bin < MEMPAGEBINS ; bin++) {
        memlkbit(&g_mem.pagebin[bin].list);
    }
    for (bin = 0 ; bin < MEMHASHITEMS ; bin++) {
        memlkbit(&g_mem.hash[bin].chain);
    }

    return;
}

static void
mempostfork(void)
{
    long bin;

    for (bin = 0 ; bin < MEMHASHITEMS ; bin++) {
        memrelbit(&g_mem.hash[bin].chain);
    }
    for (bin = 0 ; bin < MEMPAGEBINS ; bin++) {
        memrelbit(&g_mem.pagebin[bin].list);
    }
    for (bin = 0 ; bin < PTRBITS ; bin++) {
        memrelbit(&g_mem.bigbin[bin].list);
        memrelbit(&g_mem.smallbin[bin].list);
    }
    memrellk(&g_mem.heaplk);
    memrellk(&g_mem.initlk);

    return;
}

void
meminit(void)
{
    void *heap;
    long  ofs;
    void *ptr;

    memgetlk(&g_mem.initlk);
    if (g_mem.flg & MEMINITBIT) {
        memrellk(&g_mem.initlk);

        return;
    }
#if (MEMHASH) || (MEMARRAYHASH)
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhash));
#elif (MEMHUGELOCK)
    ptr = mapanon(0, MEMLVL1ITEMS * sizeof(struct memtabl0));
#else
    ptr = mapanon(0, MEMLVL1ITEMS * sizeof(struct memtab));
#endif
    if (ptr == MAP_FAILED) {

        abort();
    }
#if (MEMHASH) || (MEMARRAYHASH)
    g_mem.hash = ptr;
#else
    g_mem.tab = ptr;
#endif
    memgetlk(&g_mem.heaplk);
    heap = growheap(0);
    ofs = (1UL << PAGESIZELOG2) - ((long)heap & (PAGESIZE - 1));
    if (ofs != PAGESIZE) {
        growheap(ofs);
    }
    memrellk(&g_mem.heaplk);
    pthread_atfork(memprefork, mempostfork, mempostfork);
    g_mem.flg |= MEMINITBIT | MEMNOHEAPBIT;
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
        } else {
            memrellk(&g_mem.heaplk);
        }
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
    buf->bkt = &g_memtls->smallbin[slot];
    buf->ptrtab = (MEMPTR_T *)((MEMPTR_T)buf + membufhdrsize());

    return buf;
}

static void *
meminitsmallbuf(struct membuf *buf)
{
    MEMUWORD_T nblk = MEMBUFBLKS;
    MEMPTR_T   adr = (MEMPTR_T)buf;
    MEMPTR_T   ptr = adr + membufblkofs();

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
#if (MEMTEST)
    _memchkbuf(buf);
#endif

    return ptr;
}

static struct membuf *
memallocpagebuf(long slot, MEMUWORD_T nblk)
{
    MEMWORD_T      mapsz = mempagebufsize(slot, nblk);
    MEMPTR_T       adr;
    struct membuf *buf;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    buf = (struct membuf *)adr;
    buf->info = 0;
    memsetbufslot(buf, slot);
    buf->size = mapsz;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    buf->bkt = &g_memtls->pagebin[slot];
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
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
#if (MEMTEST)
    _memchkbuf(buf);
#endif

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
    buf->info = 0;
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
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
#if (MEMTEST)
    _memchkbuf(buf);
#endif

    return ptr;
}

void *
memgetbufblk(struct membuf *head, struct membkt *bkt, MEMUWORD_T *retndx)
{
    void          *ptr = NULL;
    MEMUWORD_T     nfree = memgetbufnfree(head);
    MEMUWORD_T     type = memgetbuftype(head);
    MEMUWORD_T     ndx;

    ndx = membufgetfree(head);
    nfree--;
    if (type != MEMPAGEBUF) {
        ptr = membufblkadr(head, ndx);
    } else {
        ptr = membufpageadr(head, ndx);
    }
    memsetbufnfree(head, nfree);
    if (!nfree) {
        if (head->next) {
            head->next->prev = NULL;
        }
        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)head->next);
        head->bkt = NULL;
        head->prev = NULL;
        head->next = NULL;
        bkt->nbuf--;
    } else {
        memrelbit(&bkt->list);
    }
    if (retndx) {
        *retndx = ndx;
    }

    return ptr;
}

#if (MEMHASH) || (MEMARRAYHASH)

static struct memhash *
memgethashitem(void)
{
    struct memhash *item = NULL;
    struct memhash *first;
    struct memhash *prev;
    struct memhash *cur;
    MEMUWORD_T      bsz;
    MEMADR_T        upval;
    long            n;

    memlkbit(&g_mem.hashbuf);
    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    if (upval) {
        item = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item->chain);
    } else {
#if (MEMBIGARRAYHASH)
        n = 4 * PAGESIZE / memhashsize();
        bsz = 4 * PAGESIZE;
#elif (MEMARRAYHASH)
        n = PAGESIZE / memhashsize();
        bsz = PAGESIZE;
#else
        n = PAGESIZE / sizeof(struct memhash);
        bsz = PAGESIZE;
#endif
        item = mapanon(0, bsz);
        if (item == MAP_FAILED) {

            abort();
        }
        first = item;
        n--;
        first++;
        cur = first;
        prev = cur;
        while (--n) {
            cur++;
            prev->chain = cur;
        }
        upval = (MEMADR_T)g_mem.hashbuf;
        upval &= ~MEMLKBIT;
        cur->chain = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)first);

        return item;
    }

    return item;
}

static void
membufhashitem(struct memhash *item)
{
    MEMADR_T upval;

#if (MEMARRAYHASH)
    item->ntab = 0;
#else
    item->adr = 0;
    item->val = 0;
#endif
    memlkbit(&g_mem.hashbuf);
    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    item->chain = (struct memhash *)upval;
    m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item);

    return;
}

#endif

#if (MEMARRAYHASH)

struct memhashitem *
memfindbuf(void *ptr, MEMWORD_T incr, MEMADR_T *keyret)
{
    MEMADR_T            adr = (MEMADR_T)ptr;
    MEMADR_T            upval;
    struct memhashitem *slot;
    long                key;
    struct memhash     *head;
    struct memhash     *blk;
    struct memhash     *prev;
    struct memhashitem *src;
    MEMADR_T            val;
    MEMUWORD_T          n;
    MEMUWORD_T          ndx;
    MEMUWORD_T          found;

    adr >>= PAGESIZELOG2;
//    key = razohash((void *)adr, sizeof(void *), MEMHASHBITS);
    key = adr & ((MEMWORD(1) << MEMHASHBITS) - 1);
    if (keyret) {
        *keyret = key;
    }
    memlkbit(&g_mem.hash[key].chain);
    upval = (MEMADR_T)g_mem.hash[key].chain;
    upval &= ~MEMLKBIT;
    head = (struct memhash *)upval;
    blk = head;
    found = 0;
    if (blk) {
        prev = NULL;
        do {
            n = blk->ntab;
            switch (n) {
#if (MEMBIGARRAYHASH)
                case 20:
                    slot = &blk->tab[19];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 19:
                    slot = &blk->tab[18];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 18:
                    slot = &blk->tab[17];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 17:
                    slot = &blk->tab[16];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 16:
                    slot = &blk->tab[15];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 15:
                    slot = &blk->tab[14];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 14:
                    slot = &blk->tab[13];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 13:
                    slot = &blk->tab[12];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 12:
                    slot = &blk->tab[11];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 11:
                    slot = &blk->tab[10];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
#endif
                case 10:
                    slot = &blk->tab[9];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 9:
                    slot = &blk->tab[8];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 8:
                    slot = &blk->tab[7];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 7:
                    slot = &blk->tab[6];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 6:
                    slot = &blk->tab[5];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 5:
                    slot = &blk->tab[4];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 4:
                    slot = &blk->tab[3];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 3:
                    slot = &blk->tab[2];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 2:
                    slot = &blk->tab[1];
                    if (slot->adr == adr) {
                        found++;
                        
                        break;
                    }
                case 1:
                    slot = &blk->tab[0];
                if (slot->adr == adr) {
                    found++;
                    
                    break;
                }
                case 0:
                default:
                    blk = blk->chain;
                    prev = blk;
            }
        } while (!found && (blk));
    }
    if (found) {
        if (incr) {
            slot->nref += incr;
#if (MEMDEBUG)
            crash(slot->nref >= 0);
#endif
            if (incr == MEMHASHDEL) {
                n--;
                if (slot->nref) {
                    if (prev) {
                        prev->chain = blk->chain;
                        blk->chain = head;
                        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                    (m_atomic_t)blk);
                    } else {
                        memrelbit(&g_mem.hash[key].chain);
                    }
                } else if (blk->ntab == 1) {
                    if (prev) {
                        prev->chain = blk->chain;
                        memrelbit(&g_mem.hash[key].chain);
                    } else {
                        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                    (m_atomic_t)blk->chain);
                    }
                    membufhashitem(blk);
                } else {
                    src = &blk->tab[n];
                    slot->nref = src->nref;
                    slot->adr = src->adr;
                    slot->val = src->val;
                    blk->ntab = n;
                    memrelbit(&g_mem.hash[key].chain);
                }
            } else {
                if (prev) {
                    prev->chain = blk->chain;
                    blk->chain = head;
                    m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                (m_atomic_t)blk);
                } else {
                    memrelbit(&g_mem.hash[key].chain);
                }
            }
        } else {
            memrelbit(&g_mem.hash[key].chain);
        }
            
        return slot;
    } else if (incr == MEMHASHADD && (head)) {
        blk = head;
        do {
            n = blk->ntab;
            if (n < MEMHASHTABITEMS) {
                slot = &blk->tab[n];
                n++;
                blk->ntab = n;

                return slot;
            }
            blk = blk->chain;
        } while (blk);
    }
    memrelbit(&g_mem.hash[key].chain);

    return NULL;
}

void *
memsetbuf(void *ptr, struct membuf *buf, MEMUWORD_T info)
{
    MEMADR_T            adr = (MEMADR_T)ptr;
    MEMADR_T            key;
    struct memhashitem *slot = memfindbuf(ptr, MEMHASHADD, &key);
    MEMADR_T            val = (MEMADR_T)buf;
    MEMADR_T            upval;
    struct memhash     *item;
    struct memhash     *chain;

    adr >>= PAGESIZELOG2;
    val |= info;
    if (slot) {
        slot->nref++;
        slot->adr = adr;
        slot->val = val;
        memrelbit(&g_mem.hash[key].chain);
    } else {
        item = memgethashitem();
        upval = (MEMADR_T)g_mem.hash[key].chain;
        slot = &item->tab[0];
        item->ntab = 1;
        slot->nref = 1;
        upval &= ~MEMLKBIT;
        slot->adr = adr;
        slot->val = val;
        chain = (struct memhash *)chain;
        item->chain = chain;
        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain, (m_atomic_t)item);
    }

    return ptr;
}

#elif (MEMHASH)

MEMADR_T
memfindbuf(void *ptr, MEMWORD_T incr, MEMADR_T *keyret)
{
    MEMADR_T        adr = (MEMADR_T)ptr;
    MEMADR_T        upval;
    long            key;
    struct memhash *head;
    struct memhash *item;
    struct memhash *prev;
    MEMADR_T        val;

    adr >>= PAGESIZELOG2;
//    key = razohash((void *)adr, sizeof(void *), MEMHASHBITS);
    key = adr & ((MEMWORD(1) << MEMHASHBITS) - 1);
    if (keyret) {
        *keyret = key;
    }
    memlkbit(&g_mem.hash[key].chain);
    upval = (MEMADR_T)g_mem.hash[key].chain;
    upval &= ~MEMLKBIT;
    head = (struct memhash *)upval;
    prev = NULL;
    item = head;
    while (item) {
        if (item->adr == adr) {
            val = item->val;
            if (incr) {
                item->nref += incr;
#if (MEMDEBUG)
                crash(item->nref >= 0);
#endif
                if (item->nref) {
                    if (prev) {
                        prev->chain = item->chain;
                        item->chain = head;
                        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                    (m_atomic_t)item);
                    } else {
                        memrelbit(&g_mem.hash[key].chain);
                    }
                } else {
                    if (prev) {
                        prev->chain = item->chain;
                        memrelbit(&g_mem.hash[key].chain);
                    } else {
                        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                    (m_atomic_t)item->chain);
                    }
                    membufhashitem(item);
                }
            } else {
                memrelbit(&g_mem.hash[key].chain);
            }
            
            return val;
        }
        prev = item;
        item = item->chain;
    }
    memrelbit(&g_mem.hash[key].chain);

    return 0;
}

void *
memsetbuf(void *ptr, struct membuf *buf, MEMUWORD_T info)
{
    MEMADR_T        adr = (MEMADR_T)ptr;
    MEMADR_T        key;
    MEMADR_T        val = memfindbuf(ptr, 1, &key);
    MEMADR_T        upval;
    struct memhash *item;

    adr >>= PAGESIZELOG2;
    if (!val) {
        item = memgethashitem();
        val = (MEMADR_T)buf;
        item->nref = 1;
        val |= info;
        item->adr = adr;
        item->val = val;
        memlkbit(&g_mem.hash[key].chain);
        upval = (MEMADR_T)g_mem.hash[key].chain;
        upval &= ~MEMLKBIT;
        item->chain = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain, (m_atomic_t)item);
    }

    return ptr;
}

#elif (MEMHUGELOCK)

void *
memsetbuf(void *ptr, struct membuf *buf, MEMUWORD_T info)
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
    long             k4;
    void            *pstk[2] = { NULL };
    
    memgetkeybits(ptr, k1, k2, k3, k4);
    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memtab));
        if (itab == MAP_FAILED) {
            memrelbit(lk1);
            
            return NULL;
        }
        pstk[0] = itab;
        g_mem.tab[k1].tab = itab;
    }
    tab = &itab[k2];
    itab = tab->tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memtab));
        if (itab == MAP_FAILED) {
            unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memtab));
            memrellk(&g_mem.tab[k1].lk);

            return NULL;
        }
        pstk[1] = itab;
        tab->tab = itab;
    }
    tab = &itab[k3];
    itab = tab->tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memitem));
        if (itab == MAP_FAILED) {
            unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memtab));
            unmapanon(pstk[1], MEMLVLITEMS * sizeof(struct memtab));
            memrellk(&g_mem.tab[k1].lk);
            
            return NULL;
        }
        tab->tab = itab;
    }
    item = (struct memitem *)&itab[k4];
    item->nref++;
    item->val = val;
    memrellk(&g_mem.tab[k1].lk);
    
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
    long            k4;

    memgetkeybits(ptr, k1, k2, k3, k4);
    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (itab) {
        tab = &itab[k2];
        itab = tab->tab;
        if (itab) {
            tab = &itab[k3];
            itab = tab->tab;
            if (itab) {
                item = (struct memitem *)&itab[k4];
                ret = item->val;
                if (rel) {
                    if (!--item->nref) {
                        item->val = 0;
                    }
                }
            }
        }
    } else {
        memrelbit(lk1);
    }
    memrellk(&g_mem.tab[k1].lk);

    return ret;
}

#else

void *
memsetbuf(void *ptr, struct membuf *buf, MEMUWORD_T info)
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
    long             k4;
    void            *pstk[2] = { NULL };
    
    memgetkeybits(ptr, k1, k2, k3, k4);
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
    if (*lk2) {
        memlkbit(lk2);
        memrelbit(lk1);
        itab = (struct memtab *)((MEMADR_T)tab->tab & ~MEMLKBIT);
    } else {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memtab));
        if (itab == MAP_FAILED) {
            unmapanon(pstk[0], MEMLVLITEMS * sizeof(struct memtab));
            memrelbit(lk1);
            
            return NULL;
        }
        pstk[1] = itab;
        lk2 = &tab->tab;
        m_syncwrite(lk2, (struct memtab *)((MEMADR_T)itab | MEMLKBIT));
        memrelbit(lk1);
    }
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
        m_syncwrite(lk1, (struct memtab *)((MEMADR_T)itab | MEMLKBIT));
        memrelbit(lk2);
    }
    item = (struct memitem *)&itab[k4];
    item->nref++;
    item->val = val;
    memrelbit(lk1);
    
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
    long            k4;

    memgetkeybits(ptr, k1, k2, k3, k4);
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
    } else {
        memrelbit(lk1);
    }
//    memrellk(&g_mem.tab[k1].lk);

    return buf;
}

#endif

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
MEMPTR_T
memputptr(struct membuf *buf, void *ptr, size_t size, size_t align, long info)
{
    MEMUWORD_T bsz = membufblksize(buf);
    MEMUWORD_T ndx = info & MEMPAGENDXMASK;
    MEMPTR_T   adr;
    
    adr = ptr;
    if (align <= MEMMINALIGN) {
        ptr = memgenptr(adr, bsz, size);
    } else if ((MEMADR_T)ptr & (align - 1)) {
        ptr = memalignptr(adr, align);
    }
    if (!ndx) {
        membufsetptr(buf, ptr, adr);
    } else {
        membufsetpage(buf, ndx, adr);
    }

    return ptr;
}

MEMPTR_T
memgetblk(long slot, long type, MEMUWORD_T size, MEMUWORD_T align)
{
    struct memtls *arn;
    MEMPTR_T       ptr = NULL;
    struct membkt *bkt;
    struct membuf *buf;
    MEMUWORD_T     info = 0;
    MEMUWORD_T     nblk = membufnblk(slot, type);
    MEMADR_T       upval = 0;
    
    arn = g_memtls;
    if (type == MEMSMALLBUF) {
        bkt = &arn->smallbin[slot];
        upval = memopenbuf(bkt);
        buf = (struct membuf *)upval;
        if (buf) {
            /* TODO */
            ptr = memgetbufblk(buf, bkt, NULL);
        } else {
            memrelbit(&bkt->list);
            bkt = &g_mem.smallbin[slot];
            upval = memopenbuf(bkt);
            buf = (struct membuf *)upval;
            if (buf) {
                /* TODO */
                ptr = memgetbufblk(buf, bkt, NULL);
            } else {
                memrelbit(&bkt->list);
                buf = memallocsmallbuf(slot);
                if (buf) {
                    bkt = &arn->smallbin[slot];
                    ptr = meminitsmallbuf(buf);
                    memlkbit(&bkt->list);
                    upval = (MEMADR_T)bkt->list;
                    upval &= ~MEMLKBIT;
                    buf->next = (struct membuf *)upval;
                    if (buf->next) {
                        buf->next->prev = buf;
                    }
                    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                } else {
                    memrelbit(&bkt->list);
                }
            }
        }
        if (ptr) {
            info = buf->info;
            if (info & MEMHEAPBIT) {
                memlkbit(&g_mem.heap);
                upval = (MEMADR_T)g_mem.heap;
                upval &= ~MEMLKBIT;
                buf->heap = (struct membuf *)upval;
                /* this unlocks the global heap (low-bit becomes zero) */
                m_syncwrite(&g_mem.heap, buf);
                memrellk(&g_mem.heaplk);
            }
            info = 0;
        }
    } else if (type == MEMPAGEBUF) {
        bkt = &arn->pagebin[slot];
        upval = memopenbuf(bkt);
        buf = (struct membuf *)upval;
        if (buf) {
            /* TODO */
            ptr = memgetbufblk(buf, bkt, &info);
        } else {
            memrelbit(&bkt->list);
            bkt = &g_mem.pagebin[slot];
            upval = memopenbuf(bkt);
            buf = (struct membuf *)upval;
            if (buf) {
                ptr = memgetbufblk(buf, bkt, &info);
            } else {
                memrelbit(&bkt->list);
                buf = memallocpagebuf(slot, nblk);
                if (buf) {
                    bkt = &arn->smallbin[slot];
                    ptr = meminitpagebuf(buf, nblk);
                    memlkbit(&bkt->list);
                    upval = (MEMADR_T)bkt->list;
                    upval &= ~MEMLKBIT;
                    buf->next = (struct membuf *)upval;
                    if (buf->next) {
                        buf->next->prev = buf;
                    }
                    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
                } else {
                    memrelbit(&bkt->list);
                }
            }
        }
    } else {
        bkt = &g_mem.bigbin[slot];
        upval = memopenbuf(bkt);
        buf = (struct membuf *)upval;
        if (buf) {
            ptr = memgetbufblk(buf, bkt, &info);
        } else {
            buf = memallocbigbuf(slot, nblk);
            if (buf) {
                ptr = meminitbigbuf(buf, nblk);
                upval = (MEMADR_T)bkt->list;
                upval &= ~MEMLKBIT;
                buf->next = (struct membuf *)upval;
                if (buf->next) {
                    buf->next->prev = buf;
                }
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf);
            } else {
                memrelbit(&bkt->list);
            }
        }
    }
#if (MEMDEBUG)
    crash(buf != NULL);
#endif
    if (ptr) {
        ptr = memputptr(buf, ptr, size, align, info);
        memsetbuf(ptr, buf, info);
    }
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

void
memputblk(void *ptr, struct membuf *buf, MEMUWORD_T info)
{
    struct membkt *bkt = buf->bkt;
    MEMUWORD_T     ndx = info & MEMPAGENDXMASK;
    MEMUWORD_T     slot = memgetbufslot(buf);
    MEMUWORD_T     nblk = memgetbufnblk(buf);
    MEMUWORD_T     nfree = memgetbufnfree(buf);
    MEMUWORD_T     type = memgetbuftype(buf);
    MEMPTR_T       adr;
    MEMADR_T       upval;
    MEMUWORD_T     id;

#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif
    if (bkt) {
        memlkbit(&bkt->list);
    }
    nfree++;
    if (type != MEMPAGEBUF) {
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
    if (nfree == nblk) {
        if (type == MEMSMALLBUF
            && bkt->nbuf >= membufnarn(slot, MEMSMALLBUF)) {
            if (buf->prev) {
                buf->prev->next = buf->next;
                memrelbit(&bkt->list);
            } else {
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t *)buf->next);
            }
            bkt = &g_mem.smallbin[slot];
            memlkbit(&bkt->list);
            upval = (MEMADR_T)bkt->list;
            upval &= ~MEMLKBIT;
            buf->next = (struct membuf *)upval;
            m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t *)buf);
        } else if (type == MEMPAGEBUF
                   && bkt->nbuf >= membufnarn(slot, MEMPAGEBUF)) {
            if (buf->prev) {
                buf->prev->next = buf->next;
                memrelbit(&bkt->list);
            } else {
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t *)buf->next);
            }
            bkt = &g_mem.pagebin[slot];
            memlkbit(&bkt->list);
            upval = (MEMADR_T)bkt->list;
            upval &= ~MEMLKBIT;
            buf->next = (struct membuf *)upval;
            m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t *)buf);
        } else if (type == MEMBIGBUF
                   && bkt->nbuf >= membufnglob(slot, MEMBIGBUF)) {
            if (buf->prev) {
                buf->prev->next = buf->next;
                memrelbit(&bkt->list);
            } else {
                m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t *)buf->next);
            }
            unmapanon(buf, buf->size);
        } else {
            memrelbit(&bkt->list);
        }
    } else if (nfree == 1 && nblk > 1) {
        if (type == MEMSMALLBUF) {
            bkt = &g_memtls->smallbin[slot];
        } else if (type == MEMPAGEBUF) {
            bkt = &g_memtls->pagebin[slot];
        } else {
            bkt = &g_mem.bigbin[slot];
        }
        memlkbit(&bkt->list);
        upval = (MEMADR_T)bkt->list;
        upval &= ~MEMLKBIT;
        buf->next = (struct membuf *)upval;
        if (buf->next) {
            buf->next->prev = buf;
        }
        buf->bkt = bkt;
        bkt->nbuf++;
        m_syncwrite((m_atomic_t *)&bkt->list, buf);
    } else {
        memrelbit(&bkt->list);
    }

    return;
}

