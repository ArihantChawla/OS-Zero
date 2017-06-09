#include <stddef.h>
#include <stdio.h>
#if (MEMDEBUG)
#include <stdio.h>
//#include <assert.h>
//#include <crash.h>
#endif
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/spin.h>
#define ZEROMTX 1
#include <zero/mtx.h>
#include <zero/mem.h>
#include <zero/hash.h>
#include <zero/valgrind.h>

static pthread_once_t               g_initonce = PTHREAD_ONCE_INIT;
static pthread_key_t                g_thrkey;
#if (!MEMDYNTLS)
THREADLOCAL struct memtls           g_memtlsdata;
#endif
THREADLOCAL volatile struct memtls *g_memtls;
struct mem                          g_mem;
#if (MEMSTAT)
struct memstat                      g_memstat;
#endif

static void
memreltls(void *arg)
{
    void                   *adr = arg;
    volatile struct membkt *src;
    volatile struct membkt *dest;
    MEMWORD_T               slot;
//    MEMWORD_T               bufsz;
    MEMWORD_T               nbuf;
//    MEMWORD_T               nb;
    struct membuf          *buf;
    struct membuf          *head;
    MEMADR_T                upval;

    if (g_memtls) {
#if 0
        bufsz = g_memtls->nbytetab[MEMSMALLBUF];
#endif
        for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
            src = &g_memtls->smallbin[slot];
#if (MEMDEADBINS)
            dest = &g_mem.deadsmall[slot];
#else
            dest = &g_mem.smallbin[slot];
#endif
            head = src->list;
            nbuf = 0;
            if (head) {
#if (MEMBUFRELMAP)
                membuffreemap(head);
#endif
                nbuf++;
                buf = head;
#if (MEMBUFRELMAP)
//                membuffreerel(buf);
#endif
                buf->tls = NULL;
                while (buf->next) {
                    nbuf++;
                    buf = buf->next;
#if (MEMBUFRELMAP)
                    membuffreemap(buf);
#endif
#if (MEMBUFRELMAP)
//                    membuffreerel(buf);
#endif
                    buf->tls = NULL;
                }
                memlkbit(&dest->list);
                upval = (MEMADR_T)dest->list;
                upval &= ~MEMLKBIT;
                head = (struct membuf *)upval;
//                buf->prev = NULL;
                buf->next = head;
                if (upval) {
                    head->prev = buf;
                }
                dest->nbuf += nbuf;
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
            }
        }
#if 0
        nb = g_mem.nbytetab[MEMSMALLBUF];
        nb += bufsz;
        g_mem.nbytetab[MEMSMALLBUF] = bufsz;
        bufsz = g_memtls->nbytetab[MEMPAGEBUF];
#endif
        for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
            src = &g_memtls->pagebin[slot];
#if (MEMDEADBINS)
            dest = &g_mem.deadpage[slot];
#else
            dest = &g_mem.pagebin[slot];
#endif
#if 0
            dest = &g_mem.pagebin[slot];
#endif
            head = src->list;
            nbuf = 0;
            if (head) {
#if (MEMBUFRELMAP)
                membuffreestk(head);
#endif
                buf = head;
                nbuf++;
                buf->tls = NULL;
                while (buf->next) {
                    nbuf++;
                    buf = buf->next;
#if (MEMBUFRELMAP)
                    membuffreestk(buf);
#endif
                    buf->tls = NULL;
                }
                memlkbit(&dest->list);
                upval = (MEMADR_T)dest->list;
                upval &= ~MEMLKBIT;
                head = (struct membuf *)upval;
//                buf->prev = NULL;
                buf->next = head;
                if (upval) {
                    head->prev = buf;
                }
                dest->nbuf += nbuf;
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
            }
        }
#if 0
        nb = g_mem.nbytetab[MEMSMALLBUF];
        nb += bufsz;
        g_mem.nbytetab[MEMSMALLBUF] = bufsz;
#endif
#if (MEM_LK_TYPE == MEM_LK_PRIO)
        priolkfin();
#endif
        if (adr) {
            unmapanon(adr, memtlssize());
        }
    }

    return;
}

#if (MEM_LK_TYPE == MEM_LK_PRIO)
static unsigned long
memgetprioval(void)
{
    unsigned long val;

    spinlk(&g_mem.priolk);
    val = g_mem.prioval;
    val++;
    val &= sizeof(long) * CHAR_BIT - 1;
    g_mem.prioval = val;
    spinunlk(&g_mem.priolk);

    return val;
}
#endif

volatile struct memtls *
meminittls(void)
{
#if (MEMDYNTLS)
    struct memtls *tls;
    struct memtls *adr;
#endif
#if (MEM_LK_TYPE == MEM_LK_PRIO)
    unsigned long  val;
#endif

    spinlk(&g_mem.initlk);
    pthread_once(&g_initonce, meminit);
#if (MEMDYNTLS)
    tls = mapanon(0, memtlssize());
    if (tls != MAP_FAILED) {
        adr = (struct memtls *)memgentlsadr((MEMWORD_T *)tls);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
        val = memgetprioval();
        priolkinit(&adr->priolkdata, val);
#endif
        pthread_setspecific(g_thrkey, tls);
        g_memtls = adr;
    }
#else /* !MEMDYNTLS */
    pthread_setspecific(g_thrkey, NULL);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
    val = memgetprioval();
    priolkinit(&g_memtlsdata.priolkdata, val);
#endif
#if (!MEMDYNTLS)
    g_memtls = &g_memtlsdata;
#endif
#endif
    spinunlk(&g_mem.initlk);

    return g_memtls;
}

static void
memprefork(void)
{
    MEMWORD_T slot;
#if (MEMHASHSUBTABS)
    MEMWORD_T ofs;
#endif

    spinlk(&g_mem.initlk);
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    memgetlk(&g_mem.heaplk);
#endif
#if (!MEMBLKHDR)
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
#if (MEMHASHSUBTABS)
            fmtxlk(&g_mem.hash[slot]->lk);
#else
            memlkbit(&g_mem.hash[slot].chain);
#endif
        }
    }
#endif
    for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
        memlkbit(&g_mem.smallbin[slot].list);
#if (MEMDEADBINS)
        memlkbit(&g_mem.deadsmall[slot].list);
#endif
    }
    for (slot = 0 ; slot < MEMBIGSLOTS ; slot++) {
        memlkbit(&g_mem.bigbin[slot].list);
    }
    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
        memlkbit(&g_mem.pagebin[slot].list);
#if (MEMDEADBINS)
        memlkbit(&g_mem.deadpage[slot].list);
#endif
    }

    return;
}

static void
mempostfork(void)
{
    MEMWORD_T slot;

    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
#if (MEMDEADBINS)
        memrelbit(&g_mem.deadpage[slot].list);
#endif
        memrelbit(&g_mem.pagebin[slot].list);
    }
    for (slot = 0 ; slot < MEMBIGSLOTS ; slot++) {
        memrelbit(&g_mem.bigbin[slot].list);
    }
    for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
#if (MEMDEADBINS)
        memrelbit(&g_mem.deadsmall[slot].list);
#endif
        memrelbit(&g_mem.smallbin[slot].list);
    }
#if (!MEMBLKHDR)
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
#if (MEMHASHSUBTABS)
            fmtxunlk(&g_mem.hash[slot]->lk);
#else
            memrelbit(&g_mem.hash[slot].chain);
#endif
        }
    }
#endif
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    memrellk(&g_mem.heaplk);
#endif
    spinunlk(&g_mem.initlk);

    return;
}

NORETURN
void
memexit(int sig)
{
#if (MEMSTAT)
    memprintstat();
#endif

    exit(sig);
}

NORETURN
void
memquit(int sig)
{
#if (MEMSTAT)
    memprintstat();
#endif
    fprintf(stderr, "CAUGHT signal %d, aborting\n", sig);
    abort();
}

void
meminit(void)
{
#if (MEMHASHSUBTABS)
    struct memhashbkt *hash;
    MEMWORD_T          ndx;
#endif
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    void              *heap;
    intptr_t           ofs;
#endif
    void              *ptr;

//    fprintf(stderr, "MEMHASHARRAYITEMS == %d\n", MEMHASHARRAYITEMS);
//    spinlk(&g_mem.initlk);
#if (MEMSIGNAL)
    signal(SIGQUIT, memexit);
    signal(SIGINT, memexit);
    signal(SIGTERM, memexit);
    signal(SIGSEGV, memquit);
    signal(SIGABRT, memquit);
#endif
    pthread_atfork(memprefork, mempostfork, mempostfork);
    pthread_key_create(&g_thrkey, memreltls);
#if (MEMSTAT)
    atexit(memprintstat);
#endif
#if (!MEMBLKHDR)
#if (MEMHASHSUBTABS)
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhashbkt));
#if (MEMSTAT)
    g_memstat.nbhashtab = MEMHASHITEMS * sizeof(struct memhashbkt);
#endif
#else
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhashlist));
#if (MEMSTAT)
    g_memstat.nbhashtab = MEMHASHITEMS * sizeof(struct memhashlist);
#endif
#endif
    if (ptr == MAP_FAILED) {
        crash(ptr == MAP_FAILED);
    }
#if (MEMHASHSUBTABS)
    hash = ptr;
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhashbkt *));
#if (MEMSTAT)
    g_memstat.nbhashtab = MEMHASHITEMS * sizeof(struct memhashbkt *);
#endif
    g_mem.hash = ptr;
    for (ndx = 0 ; ndx < MEMHASHITEMS ; ndx++) {
        g_mem.hash[ndx] = hash;
        hash++;
    }
#endif
#endif /* !MEMBLKHDR */
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
//    memgetlk(&g_mem.heaplk);
    heap = growheap(0);
    ofs = (1UL << PAGESIZELOG2) - ((long)heap & (PAGESIZE - 1));
    if (ofs != PAGESIZE) {
        growheap(ofs);
    }
//    memrellk(&g_mem.heaplk);
#endif
    g_mem.flg |= MEMINITBIT;
//    spinunlk(&g_mem.initlk);

    return;
}

static struct membuf *
memallocsmallbuf(MEMWORD_T slot, MEMWORD_T nblk)
{
    MEMPTR_T       adr = SBRK_FAILED;
    MEMWORD_T      bufsz = memsmallbufsize(slot, nblk);
    MEMWORD_T      flg = 0;
    struct membuf *buf;

#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    if (!(g_mem.flg & MEMNOHEAPBIT)) {
        /* try to allocate from heap (sbrk()) */
        memgetlk(&g_mem.heaplk);
        adr = growheap(bufsz);
        flg = MEMHEAPBIT;
        if (adr != SBRK_FAILED) {
#if (MEMSTAT)
            g_memstat.nbheap += bufsz;
#endif
            g_mem.flg |= MEMHEAPBIT;
        } else {
            g_mem.flg |= MEMNOHEAPBIT;
            memrellk(&g_mem.heaplk);
        }
    }
#endif /* !MEMNOSBRK */
    if (adr == SBRK_FAILED) {
        /* sbrk() failed or was skipped, let's try mmap() */
        adr = mapanon(0, bufsz);
        if (adr == MAP_FAILED) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif

            return NULL;
        }
#if (MEMSTAT)
        g_memstat.nbsmall += bufsz;
//        g_memstat.nbmap += bufsz;
#endif
    }
    buf = (struct membuf *)adr;
    adr += membufmapsize();
    buf->base = adr;
    buf->flg = flg;    // possible MEMHEAPBIT
    meminitbufslot(buf, slot);
    meminitbufnblk(buf, nblk);
    meminitbuftype(buf, MEMSMALLBUF);
    buf->size = bufsz;
#if (MEMTEST)
    _memchkbuf(buf, MEMSMALLBUF, nblk, flg, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitsmallbuf(struct membuf *buf,
                MEMWORD_T slot,
                MEMWORD_T size, MEMWORD_T align,
                MEMWORD_T nblk)
{
    MEMPTR_T  ptr = buf->base;
    MEMWORD_T bsz = MEMWORD(1) << slot; 
    MEMWORD_T tmp = 0;
    MEMADR_T  pad;

    /* initialise free-bitmaps */
    membufinitmap(buf, nblk);
    membufscanblk(buf, &tmp);
    VALGRINDMKPOOL(ptr, 0, 0);
    ptr = memcalcadr(ptr, size, bsz, align, &pad);
#if 0
    ptr = memalignptr(ptr, align);
#endif
    memsetblk(ptr, buf, 0, pad);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

static struct membuf *
memallocpagebuf(MEMWORD_T slot, MEMWORD_T nblk)
{
    MEMWORD_T      mapsz = mempagebufsize(slot, nblk);
    MEMPTR_T       adr;
    struct membuf *buf;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    buf = (struct membuf *)adr;
    adr += membufhdrsize();
    buf->base = adr;
    buf->flg = 0;
    meminitbufslot(buf, slot);
    meminitbufnblk(buf, nblk);
    meminitbuftype(buf, MEMPAGEBUF);
    buf->size = mapsz;
#if (MEMSTAT)
    g_memstat.nbpage += mapsz;
//    g_memstat.nbmap += mapsz;
#endif
#if (MEMTEST)
    _memchkbuf(buf, MEMPAGEBUF, nblk, 0, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitpagebuf(struct membuf *buf,
               MEMWORD_T slot,
               MEMWORD_T size, MEMWORD_T align,
               MEMWORD_T nblk)
{
    MEMPTR_T  ptr = buf->base;
    MEMWORD_T bsz = PAGESIZE + slot * PAGESIZE;
    MEMWORD_T tmp = 0;
    MEMADR_T  pad;

    /* initialise free-bitmaps */
    membufinitmap(buf, nblk);
    membufscanblk(buf, &tmp);
    VALGRINDMKPOOL(ptr, 0, 0);
    ptr = memcalcadr(ptr, size, bsz, align, &pad);
    memsetblk(ptr, buf, 0, pad);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

static struct membuf *
memallocbigbuf(MEMWORD_T slot, MEMWORD_T nblk)
{
    MEMWORD_T      mapsz = membigbufsize(slot, nblk);
    MEMPTR_T       adr;
    struct membuf *buf;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    buf = (struct membuf *)adr;
    adr += membufhdrsize();
    buf->base = adr;
    buf->flg = 0;
    meminitbufslot(buf, slot);
    meminitbufnblk(buf, nblk);
    meminitbuftype(buf, MEMBIGBUF);
#if (MEMSTAT)
    g_memstat.nbbig += mapsz;
#endif
    buf->size = mapsz;
#if (MEMTEST)
    _memchkbuf(buf, MEMBIGBUF, nblk, 0, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitbigbuf(struct membuf *buf,
              MEMWORD_T slot,
              MEMWORD_T size, MEMWORD_T align,
              MEMWORD_T nblk)
{
    MEMPTR_T  ptr = buf->base;
    MEMWORD_T bsz = MEMWORD(1) << slot;
    MEMWORD_T tmp = 0;
    MEMADR_T  pad;

    /* initialise free-bitmaps */
    membufinitmap(buf, nblk);
    membufscanblk(buf, &tmp);
    VALGRINDMKPOOL(buf, 0, 0);
    ptr = memcalcadr(ptr, size, bsz, align, &pad);
    memsetblk(ptr, buf, 0, pad);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

#if (!MEMBLKHDR)

static void
meminithashitem(MEMPTR_T data)
{
    struct memhash *item = (struct memhash *)data;
    MEMWORD_T      *adr;

    data += offsetof(struct memhash, data);
    item->chain = NULL;
    adr = (MEMWORD_T *)data;
    adr = memgenhashtabadr(adr);
    item->ntab = 0;
    item->tab = (struct memhashitem *)adr;
    item->chain = NULL;

    return;
}

static struct memhash *
memgethashitem(void)
{
    struct memhash *item = NULL;
    MEMPTR_T        first;
    struct memhash *prev;
    struct memhash *cur;
    MEMPTR_T        next;
    MEMWORD_T       bsz;
    MEMADR_T        upval;
    long            n;

    memlkbit(&g_mem.hashbuf);
    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    if (upval) {
        item = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item->chain);
//        meminithashitem(item);
    } else {
#if (MEMHASHSUBTABS)
        n = 32 * PAGESIZE / memhashsize();
        bsz = 32 * PAGESIZE;
#elif (MEMBIGHASHTAB)
        n = 64 * PAGESIZE / memhashsize();
        bsz = 64 * PAGESIZE;
#elif (MEMSMALLHASHTAB)
        n = 16 * PAGESIZE / memhashsize();
        bsz = 16 * PAGESIZE;
#elif (MEMTINYHASHTAB)
        n = 4 * PAGESIZE / memhashsize();
        bsz = 4 * PAGESIZE;
#else
        n = 8 * PAGESIZE / memhashsize();
        bsz = 8 * PAGESIZE;
#endif
#if (MEMSTAT)
        g_memstat.nbhash += bsz;
#endif
        item = mapanon(0, bsz);
        first = (MEMPTR_T)item;
        if (item == MAP_FAILED) {

            abort();
        }
        meminithashitem(first);
//        upval = (MEMADR_T)g_mem.hashbuf;
        first += memhashsize();
//        upval &= ~MEMLKBIT;
        next = first;
        while (--n) {
            prev = (struct memhash *)next;
            meminithashitem(next);
            next += memhashsize();
            prev->chain = (struct memhash *)next;
        }
        cur = (struct memhash *)prev;
        cur->chain = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)first);
    }

    return item;
}

#if defined(MEMHASHSUBTABS) && (MEMHASHSUBTABS)
static void
meminithashbkt(struct memhashbkt *bkt)
{
    struct memhash **item = &bkt->tab[0];
    MEMWORD_T        ndx;

    for (ndx = 0 ; ndx < (1L << (MEMHASHSUBTABSHIFT + PAGESIZELOG2)) ; ndx++) {
        item[ndx] = memgethashitem();
    }

    return;
}
#endif

#if defined(MEMHASHNREF) && (MEMHASHNREF)
static void
membufhashitem(struct memhash *item)
{
    MEMADR_T upval;

#if 0
    if (item->itab){
        memrelhashsubtab(item);
    }
#endif
    memlkbit(&g_mem.hashbuf);
    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    item->chain = (struct memhash *)upval;
    m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item);

    return;
}
#endif

MEMADR_T
membufop(MEMPTR_T ptr, MEMWORD_T op,
         struct membuf *buf, MEMWORD_T id)
{
    volatile struct memtls *tls;
    MEMPTR_T                adr = ptr;
#if (MEMHASHSUBTABS)
    MEMADR_T                page = ((MEMADR_T)adr
                                    >> (PAGESIZELOG2 + MEMHASHSUBTABSHIFT));
    MEMADR_T                ofs =  (((MEMADR_T)adr >> PAGESIZELOG2)
                                    & (MEMHASHSUBTABITEMS - 1));
    MEMUWORD_T              key = memhashptr(page) & (MEMHASHITEMS - 1);
#else
    MEMADR_T                page = ((MEMADR_T)adr >> PAGESIZELOG2);
    MEMUWORD_T              key = memhashptr(page) & (MEMHASHITEMS - 1);
#endif
    MEMADR_T                desc = (MEMADR_T)buf;
    MEMADR_T                upval;
    MEMADR_T                val;
    struct memhash         *blk;
    struct memhash         *prev;
    struct memhashitem     *slot;
    struct memhashitem     *src;
    struct memhashitem     *dest;
    MEMWORD_T              *cnt;
//    volatile struct memtls *tls;
    MEMWORD_T               type;
    MEMWORD_T               lim;
    MEMUWORD_T              n;
    struct memhashitem     *item;
    MEMADRDIFF_T            mask;
    
//    fprintf(stderr, "LOCK: %lx\n", key);
#if (!MEMHASHSUBTABS)
    memlkbit(&g_mem.hash[key].chain);
    upval = (MEMADR_T)g_mem.hash[key].chain;
#else
    fmtxlk(&g_mem.hash[key]->lk);
    upval = (MEMADR_T)g_mem.hash[key]->tab[ofs];
//    upval &= ~MEMLKBIT;
    if (!upval) {
        if (op != MEMBUFADD) {
            fmtxunlk(&g_mem.hash[key]->lk);
            
            return MEMBUFNOTFOUND;
        } else {
            blk = memgethashitem();
            if (!blk) {
                
                return MEMBUFNOTFOUND;
            }
            desc |= id;
            slot = &blk->tab[0];
#if defined(MEMHASHNREF) && (MEMHASHNREF)
            slot->nref = 1;
#endif
#if defined(MEMHASHNACT) && (MEMHASHNACT)
            slot->nact = 1;
#endif
            slot->adr = page;
            blk->ntab = 1;
            slot->val = desc;
#if (MEMDEBUG) && 0
            crash(slot == NULL);
#endif
//                        fprintf(stderr, "REL: %lx\n", key);
            blk->chain = NULL;
            g_mem.hash[key]->tab[ofs] = blk;
            fmtxunlk(&g_mem.hash[key]->lk);
            
            return desc;
        }
    }
#endif /* MEMHASHSUBTABS */
    dest = NULL;
    prev = NULL;
#if (!MEMHASHSUBTABS)
    upval &= ~MEMLKBIT;
#endif
    desc |= id;
    slot = NULL;
    blk = (struct memhash *)upval;
    if (blk) {
        do {
            lim = blk->ntab;
            src = blk->tab;
            if (lim != MEMHASHARRAYITEMS && !dest) {
                cnt = &blk->ntab;
                dest = &src[lim];
            }
            do {
                n = min(lim, 16);
                switch (n) {
                    /*
                     * if found, the mask will be -1; all 1-bits), and val will
                     * be the item address
                     * if not found, the mask will be 0 and so will val/slot
                     */
                    case 16:
                        item = &src[15];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 15:
                        item = &src[14];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 14:
                        item = &src[13];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 13:
                        item = &src[12];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                        if (slot) {
                            
                            break;
                        }
                    case 12:
                        item = &src[11];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 11:
                        item = &src[10];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 10:
                        item = &src[9];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 9:
                        item = &src[8];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val; 
                        if (slot) {
                            
                            break;
                        }
                    case 8:
                        item = &src[7];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 7:
                        item = &src[6];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 6:
                        item = &src[5];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 5:
                        item = &src[4];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                        if (slot) {
                            
                            break;
                        }
                    case 4:
                        item = &src[3];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 3:
                        item = &src[2];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 2:
                        item = &src[1];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                    case 1:
                        item = &src[0];
                        mask = -(item->adr == page);
                        val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                        slot = (struct memhashitem *)val;
                        if (slot) {
                            
                            break;
                        }
                    case 0:
                    default:
                        
                        break;
                }
                lim -= n;
                src += n;
            } while ((lim) && !slot);
            if (slot) {
                desc = slot->val;
                
                break;
            } else {
                prev = blk;
                blk = blk->chain;
            }
        } while ((blk) && !slot);
    }
    if (!slot) {
        if (op == MEMBUFDEL || op == MEMBUFCHK) {
//            fprintf(stderr, "REL: %lx\n", key);
#if (MEMHASHSUBTABS)
            fmtxunlk(&g_mem.hash[key]->lk);
#else
            memrelbit(&g_mem.hash[key].chain);
#endif
            
            return MEMBUFNOTFOUND;
        } else if (dest) {
            n = *cnt;
            dest->adr = page;
            n++;
            dest->val = desc;
            *cnt = n;
#if (MEMHASHSUBTABS)
            fmtxunlk(&g_mem.hash[key]->lk);
#else
            memrelbit(&g_mem.hash[key].chain);
#endif
            
            return desc;
        } else {
//            desc = (MEMADR_T)buf;
            blk = (struct memhash *)upval;
            if (blk) {
                prev = NULL;
                do {
                    n = blk->ntab;
                    if (n < MEMHASHARRAYITEMS) {
                        slot = &blk->tab[n];
                        n++;
#if defined(MEMHASHNREF) && (MEMHASHNREF)
                        slot->nref = 1;
#endif
#if defined(MEMHASHNACT) && (MEMHASHNACT)
                        slot->nact = 1;
#endif
                        slot->adr = page;
                        blk->ntab = n;
                        slot->val = desc;
#if (MEMDEBUG)
                        crash(slot == NULL);
#endif
//                        fprintf(stderr, "REL: %lx\n", key);
                        if (prev) {
                            prev->chain = blk->chain;
                            blk->chain = (struct memhash *)upval;
#if (MEMHASHSUBTABS)
                            g_mem.hash[key]->tab[ofs]->chain = blk;
#else
                            g_mem.hash[key].chain = blk;
#endif
                        }
#if (MEMHASHSUBTABS)
                        fmtxunlk(&g_mem.hash[key]->lk);
#else
                        memrelbit(&g_mem.hash[key].chain);
#endif
                        
                        return desc;
                    }
                    prev = blk;
                    blk = blk->chain;
                } while (blk);
            }
#if (MEMSTAT)
            g_memstat.nhashitem++;
#endif
            blk = memgethashitem();
            slot = blk->tab;
            blk->ntab = 1;
#if defined(MEMHASHNREF) && (MEMHASHNREF)
            slot->nref = 1;
#endif
#if defined(MEMHASHNACT) && (MEMHASHNACT)
            slot->nact = 1;
#endif
            blk->chain = (struct memhash *)upval;
#if (MEMSTAT)
            if (!upval) {
                g_memstat.nhashchain++;
            }
#endif
            slot->adr = page;
            slot->val = desc;
//            fprintf(stderr, "REL: %lx\n", key);
#if (MEMHASHSUBTABS)
            g_mem.hash[key]->tab[ofs]->chain = blk;
            fmtxunlk(&g_mem.hash[key]->lk);
#else
            g_mem.hash[key].chain = blk;
            memrelbit(&g_mem.hash[key].chain);
#endif
#if (MEMDEBUG) && 0
            crash(desc == 0);
#endif
            
            return desc;
        }
    } else {
#if defined(MEMHASHNACT) && (MEMHASHNACT)
        slot->nact++;
#endif
        if (op == MEMBUFDEL) {
            id = desc & MEMPAGEIDMASK;
            desc &= ~MEMPAGEIDMASK;
            buf = (struct membuf *)desc;
//            tls = buf->tls;
            type = memgetbuftype(buf);
            VALGRINDPOOLFREE(buf->base, ptr);
            if (type != MEMPAGEBUF) {
//                adr = membufgetadr(buf, ptr);
                id = membufblkid(buf, ptr);
            }
            tls = buf->tls;
            if (tls && tls != g_memtls) {
                membufsetrel(buf, id);
            } else {
                memrelblk(buf, id);
            }
#if defined(MEMHASHNREF) && (MEMHASHNREF)
            slot->nref--;
            n = blk->ntab;
            if (!slot->nref) {
                if (n == 1) {
//                    fprintf(stderr, "REL: %lx\n", key);
                    if (prev) {
                        prev->chain = blk->chain;
                    } else {
#if (MEMHASHSUBTABS)
                        g_mem.hash[key]->tab[ofs].chain = blk;
#else
                        g_mem.hash[key].chain = blk;
#endif
                    }
                    membufhashitem(blk);
#if (MEMHASHSUBTABS)
                    fmtxunlk(&g_mem.hash[key]->lk);
#else
                    memrelbit(&g_mem.hash[key].chain);
#endif
                    
                    return desc;
                } else {
                    n--;
                    src = &blk->tab[n];
                    slot->nref = src->nref;
#if defined(MEMHASHNACT) && (MEMHASHNACT)
                    slot->nact = src->nact;
#endif
                    slot->adr = src->adr;
                    slot->val = src->val;
                    src->adr = 0;
                    src->val = 0;
                    blk->ntab = n;
                }
            }
#endif /* MEMHASHNREF */
#if (MEMDEBUG) && 0
            crash(desc == 0);
#endif
//            fprintf(stderr, "REL: %lx\n", key);
#if (MEMHASHSUBTABS)
            fmtxunlk(&g_mem.hash[key]->lk);
#else
            memrelbit(&g_mem.hash[key].chain);
#endif
            
            return desc;
#if defined(MEMHASHNREF) && (MEMHASHNREF)
        } else if (op == MEMBUFADD) {
            slot->nref++;
#endif
        }
    }
//    fprintf(stderr, "REL: %lx\n", key);
    if (prev) {
        prev->chain = blk->chain;
        blk->chain = (struct memhash *)upval;
    }
#if (MEMHASHSUBTABS)
    fmtxunlk(&g_mem.hash[key]->lk);
#else
    memrelbit(&g_mem.hash[key].chain);
#endif
#if (MEMDEBUG) && 0
    crash(desc == 0);
#endif
    
    return desc;
}

#else /* MEMBLKHDR */

MEMADR_T
membufop(MEMPTR_T ptr, MEMWORD_T op, struct membuf *buf, MEMWORD_T id,
         MEMADR_T pad)
{
    MEMADR_T          val = (MEMADR_T)buf;
    MEMWORD_T         type;
    struct memblkhdr *hdr;

    if (op == MEMBUFDEL) {
        val = memgetblkdesc(ptr);
        memputblkdesc(ptr, 0);
        memputblkpad(ptr, 0);
    } else if (op == MEMBUFADD) {
        type = buf->type;
        if (type != MEMSMALLBUF) {
            val |= id;
        }
        memputblkdesc(ptr, val);
        memputblkpad(ptr, pad);
    } else if (op == MEMBUFCHK) {
        val = memgetblkdesc(ptr);
    } else {
        /* op == MEMBUFGETPAD */
        val = memgetblkpad(ptr);
    }

    return val;
}

#endif

#if 0
MEMPTR_T
memsetbuf(MEMPTR_T ptr, struct membuf *buf, MEMWORD_T id, MEMADR_T pad)
{
    MEMADR_T  desc = membufop(ptr, MEMBUFADD, buf, id, pad);
    
    return (MEMPTR_T)desc;
}
#endif

MEMPTR_T
memgetblktls(MEMWORD_T type, MEMWORD_T slot, MEMWORD_T size, MEMWORD_T align)
{
    volatile struct membkt  *bkt;
    struct membuf           *buf;
    MEMPTR_T                 ptr;
    MEMPTR_T                 adr;
    MEMWORD_T                bsz;
    MEMADR_T                 pad;
    MEMWORD_T                nfree;
#if (MEMDEBUG)
    MEMWORD_T                nblk;
#endif
    MEMWORD_T                q;
    MEMUWORD_T               id;

    if (type == MEMSMALLBUF) {
        bsz = MEMWORD(1) << slot;
        bkt = &g_memtls->smallbin[slot];
    } else if (type == MEMPAGEBUF) {
        bsz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_memtls->pagebin[slot];
    } else {
        bsz = MEMWORD(1) << slot;
        bkt = &g_memtls->bigbin[slot];
    }
//    memprintbufstk(buf, "MEMGETBLKTLS\n");
    buf = bkt->list;
    nfree = 0;
    if (!buf) {

        return NULL;
    }
    id = membufscanblk(buf, &nfree);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
#if defined(MEMBUFSTACK) && (MEMBUFSTACK)
    crash(nfree >= 0 && nfree < nblk);
#else
    crash(nfree <= 0 && nfree >= nblk);
#endif
#endif
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(buf, id);
    } else {
        adr = membufpageadr(buf, id);
    }
//            ptr = memcalcadr(buf, adr, size, align, id);
    ptr = memcalcadr(adr, size, bsz, align, &pad);
    if (type != MEMPAGEBUF) {
        memsetblk(ptr, buf, 0, pad);
    } else {
        memsetblk(ptr, buf, id, pad);
    }
    VALGRINDPOOLALLOC(buf->base, ptr, size);
    if (!nfree) {
        /* buf shall be disconnected from all lists */
        if (buf->next) {
            buf->next->prev = NULL;
        }
        bkt->nbuf--;
        bkt->list = buf->next;
#if (!MEMEMPTYTLS)
        buf->tls = NULL;
#endif
#if 0
        buf->prev = NULL;
        buf->next = NULL;
#endif
    }
    
    return ptr;
}

#if (MEMDEADBINS)
MEMPTR_T
memgetblkdead(MEMWORD_T type, MEMWORD_T slot, MEMWORD_T size, MEMWORD_T align)
{
    MEMPTR_T                ptr = NULL;
    volatile struct memtls *tls;
    volatile struct membkt *bkt;
    struct membuf          *buf;
    MEMPTR_T                adr;
    MEMWORD_T               bsz;
    MEMADR_T                pad;
    MEMWORD_T               nfree;
#if (MEMDEBUG)
    MEMWORD_T               nblk;
#endif
    MEMUWORD_T              id;
    MEMADR_T                upval;

    if (type == MEMSMALLBUF) {
        bsz = MEMWORD(1) << slot;
        bkt = &g_mem.deadsmall[slot];
    } else {
        bsz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_mem.deadpage[slot];
    }
#if (MEMBKTLOCK)
    memlkbkt(bkt);
    buf = bkt->list;
#else
    memlkbit(&bkt->list);
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    buf = (struct membuf *)upval;
#endif
    if (!buf) {
        memrelbit(&bkt->list);
        
        return NULL;
    }
#if (MEMBUFRELMAP) && 0
    membuffreerel(buf);
#endif
    id = membufgetfree(buf);
    nfree = memgetbufnfree(buf);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
    crash(nfree >= 0 && nfree < nblk);
#endif
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(buf, id);
    } else {
        adr = membufpageadr(buf, id);
    }
//            ptr = memcalcadr(buf, adr, size, align, id);
    ptr = memcalcadr(adr, size, bsz, align, &pad);
    if (type != MEMPAGEBUF) {
        memsetblk(ptr, buf, 0, pad);
    } else {
        memsetblk(ptr, buf, id, pad);
    }
    VALGRINDPOOLALLOC(buf->base, ptr, size);
    /* buf shall be disconnected from all lists */
    if (buf->next) {
        buf->next->prev = NULL;
    }
    bkt->nbuf--;
#if (MEMBKTLOCK)
    bkt->list = buf->next;
    memunlkbkt(bkt);
#else
    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf->next);
#endif
    if (nfree) {
        tls = g_memtls;
        buf->prev = NULL;
        if (type == MEMSMALLBUF) {
            bkt = &tls->smallbin[slot];
        } else {
            bkt = &tls->pagebin[slot];
        }
        buf->next = bkt->list;
        bkt->nbuf++;
        buf->tls = tls;
        bkt->list = buf;
    } else {
        buf->tls = NULL;
#if 0
        buf->prev = NULL;
        buf->next = NULL;
#endif
    }        

    return ptr;
}
#endif

MEMPTR_T
memgetblkglob(MEMWORD_T type, MEMWORD_T slot, MEMWORD_T size, MEMWORD_T align)
{
    volatile struct memtls *tls;
    volatile struct membkt *bkt;
    struct membuf          *buf;
    MEMPTR_T                ptr;
    MEMPTR_T                adr;
    MEMWORD_T               bsz;
    MEMADR_T                pad;
    MEMWORD_T               nfree;
#if (MEMDEBUG)
    MEMWORD_T               nblk;
#endif
    MEMWORD_T               id;
    MEMADR_T                upval;

    if (type == MEMSMALLBUF) {
        bsz = MEMWORD(1) << slot;
        bkt = &g_mem.smallbin[slot];
    } else if (type == MEMPAGEBUF) {
        bsz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_mem.pagebin[slot];
    } else {
        bsz = MEMWORD(1) << slot;
        bkt = &g_mem.bigbin[slot];
    }
//    memprintbufstk(buf, "MEMGETBLKGLOB\n");
#if (MEMBKTLOCK)
    memlkbkt(bkt);
    buf = bkt->list;
    nfree = 0;
#else
    memlkbit(&bkt->list);
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    buf = (struct membuf *)upval;
#endif
    if (!buf) {
        memrelbit(&bkt->list);

        return NULL;
    }
    id = membufscanblk(buf, &nfree);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
#if defined(MEMBUFSTACK) && (MEMBUFSTACK)
    crash(nfree >= 0 && nfree < nblk);
#else
    crash(nfree <= 0 && nfree >= nblk);
#endif
#endif
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(buf, id);
    } else {
        adr = membufpageadr(buf, id);
    }
//    ptr = memcalcadr(buf, adr, size, align, id);
    ptr = memcalcadr(adr, size, bsz, align, &pad);
    memsetblk(ptr, buf, id, pad);
#if (MEMTEST) && 0
    _memchkptr(buf, ptr);
#endif
    VALGRINDPOOLALLOC(buf->base, ptr, size);
    if (!nfree) {
        /* buf shall be disconnected from all lists */
        if (buf->next) {
            buf->next->prev = NULL;
        }
        bkt->nbuf--;
#if (MEMBKTLOCK)
        bkt->list = buf->next;
        memunlkbkt(bkt);
#else
        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf->next);
#endif
//        m_clrbit((m_atomic_t *)&buf->info, MEMBUFGLOBBITID);
#if 0
        buf->prev = NULL;
        buf->next = NULL;
#endif
    } else {
        tls = g_memtls;
        buf->prev = NULL;
        if (buf->next) {
            buf->next->prev = NULL;
        }
#if (MEMBKTLOCK)
        bkt->list = buf->next;
        memunlkbkt(bkt);
#else
        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf->next);
#endif
        if (type == MEMSMALLBUF) {
            bkt = &tls->smallbin[slot];
        } else {
            bkt = &tls->pagebin[slot];
        }
        buf->next = bkt->list;
        bkt->nbuf++;
        buf->tls = tls;
        bkt->list = buf;
    }

    return ptr;
}

MEMPTR_T
memgetblk(MEMWORD_T slot, MEMWORD_T type, MEMWORD_T size, MEMWORD_T align)
{
    volatile struct memtls    *tls;
    volatile struct membkt    *dest;
    MEMPTR_T                   ptr;
    MEMWORD_T                  nblk;
    MEMWORD_T                  flg;
    struct membuf             *buf;
    struct membuf             *head;
    MEMADR_T                   upval;
    
    ptr = memgetblktls(type, slot, size, align);
#if (MEMDEADBINS)
    if (!ptr) {
        ptr = memgetblkdead(type, slot, size, align);
    }
#endif
    if (!ptr) {
        ptr = memgetblkglob(type, slot, size, align);
    }
    if (!ptr) {
        nblk = memgetnbufblk(type, slot);
        if (type == MEMSMALLBUF) {
            buf = memallocsmallbuf(slot, nblk);
            if (buf) {
                ptr = meminitsmallbuf(buf, slot, size, align, nblk);
            }
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
            if (ptr) {
                flg = memgetbufheapflg(buf);
                if (flg & MEMHEAPBIT) {          
                    memlkbit(&g_mem.heap);
                    upval = (MEMADR_T)g_mem.heap;
                    upval &= ~MEMLKBIT;
                    buf->heap = (struct membuf *)upval;
                    /* unlocks the global heap (low-bit becomes zero) */
                    m_syncwrite((m_atomic_t *)&g_mem.heap, (m_atomic_t)buf);
                    memrellk(&g_mem.heaplk);
                }
            }
#endif /* !MEMNOSBRK */
        } else if (type == MEMPAGEBUF) {
            buf = memallocpagebuf(slot, nblk);
            if (buf) {
                ptr = meminitpagebuf(buf, slot, size, align, nblk);
            }
        } else {
            buf = memallocbigbuf(slot, nblk);
            if (buf) {
                ptr = meminitbigbuf(buf, slot, size, align, nblk);
            }
        }
        if (ptr && nblk > 1) {
            tls = g_memtls;
            if (type == MEMSMALLBUF) {
                dest = &tls->smallbin[slot];
            } else if (type == MEMPAGEBUF) {
                dest = &tls->pagebin[slot];
            } else {
                dest = &tls->bigbin[slot];
            }
            head = dest->list;
            buf->prev = NULL;
            if (head) {
                head->prev = buf;
            }
            buf->next = head;
            dest->nbuf++;
            buf->tls = tls;
                dest->list = buf;
        }
    }

    return ptr;
}

void
memdequeuebuftls(struct membuf *buf, volatile struct membkt *src)
{
    struct membuf *head = src->list;
    
    if ((buf->prev) && (buf->next)) {
        buf->prev->next = buf->next;
        buf->next->prev = buf->prev;
    } else if (buf->prev) {
        buf->prev->next = NULL;
    } else if (buf->next) {
        buf->next->prev = NULL;
        head = buf->next;
    } else {
        head = NULL;
    }
    buf->tls = NULL;
    src->nbuf--;
#if (MEMBKTLOCK)
    src->list = head;
    memunlkbkt(src);
#else
    m_syncwrite((m_atomic_t *)&src->list, head);
#endif

    return;
}

void
memdequeuebufglob(struct membuf *buf, volatile struct membkt *src)
{
    struct membuf *head;
    MEMADR_T       upval;

    upval = (MEMADR_T)src->list;
    upval &= ~MEMLKBIT;
    if ((buf->prev) && (buf->next)) {
        buf->prev->next = buf->next;
        buf->next->prev = buf->prev;
        head = (struct membuf *)upval;
    } else if (buf->prev) {
        buf->prev->next = NULL;
        head = (struct membuf *)upval;
    } else if (buf->next) {
        buf->next->prev = NULL;
        head = buf->next;
    } else {
        head = NULL;
    }
    src->nbuf--;
#if (MEMBKTLOCK)
    src->list = head;
    memunlkbkt(src);
#else
    m_syncwrite((m_atomic_t *)&src->list, (m_atomic_t)head);
#endif

    return;
}

void
memrelblk(struct membuf *buf, MEMWORD_T id)
{
    volatile struct memtls *tls;
    volatile struct membkt *gbkt;
    volatile struct membkt *tbkt;
    struct membuf          *head;
    MEMWORD_T               type;
    MEMWORD_T               nfree;
    MEMWORD_T               nblk;
    MEMWORD_T               slot;
    MEMWORD_T               lim;
    MEMWORD_T               q;
    MEMADR_T                upval;
    MEMWORD_T               val;

    tls = buf->tls;
#if 0
    if (tls && tls != g_memtls) {
        membufsetrel(buf, id);


        return;
    }
#endif
    type = memgetbuftype(buf);
    slot = memgetbufslot(buf);
    if (type == MEMSMALLBUF) {
        tbkt = &g_memtls->smallbin[slot];
        gbkt = &g_mem.smallbin[slot];
    } else if (type == MEMPAGEBUF) {
        tbkt = &g_memtls->pagebin[slot];
        gbkt = &g_mem.pagebin[slot];
    } else {
        tbkt = NULL;
        gbkt = &g_mem.bigbin[slot];
    }
    if (!tls) {
#if (MEMBKTLOCK)
        memlkbkt(gbkt);
#else
        memlkbit(&gbkt->list);
#endif
    }
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    nfree = memgetbufnfree(buf);
#endif
    nblk = memgetbufnblk(buf);
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    nfree++;
#endif
#if (MEMTEST) && 0
    _memchkptr(buf, adr);
#endif
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    setbit(buf->freemap, id);
    memsetbufnfree(buf, nfree);
#else
    nfree = membufpushblk(buf, id);
#if 0
    if (type == MEMSMALLBUF) {
        nfree = membufpushblk(buf, id);
    } else {
        nfree = membufputblk(buf, id);
    }
#endif
#endif
    if (nfree != 1 && nfree != nblk) {
        /* buffer not totally free or allocated */

        return;
    } else if (nfree == 1) {
#if (MEMEMPTYTLS)
        head = tbkt->list;
        if (head) {
            head->prev = buf;
        }
        buf->next = head;
        tbkt->nbuf++;
        tbkt->list = buf;
#if (!MEMDEADBINS)
        if (type == MEMSMALLBUF) {
            membuffreestk(buf);
        } else {
            membuffreemap(buf);
        }
#endif
#else /* !MEMEMPTYTLS */
#if (MEMBKTLOCK)
        memlkbkt(gbkt);
#else
        memlkbit(&gbkt->list);
#endif
        /* add buffer in front of global list */
#if (MEMBKTLOCK)
        buf->prev = NULL;
        head = gbkt->list;
#else
        upval = (MEMADR_T)gbkt->list;
        buf->prev = NULL;
        upval &= ~MEMLKBIT;
        head = (struct membuf *)upval;
#endif
        if (head) {
            head->prev = buf;
        }
        buf->next = head;
        gbkt->nbuf++;
//        m_setbit(&buf->info, MEMBUFGLOBBITID);
            /* this will unlock the list (set the low-bit to zero) */
        m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
#endif /* MEMEMPTYTLS */
        
        return;
    } else if (nfree != nblk && !tls) {
#if (MEMBKTLOCK)
        memunlkbkt(gbkt);
#else
        memrelbit(&gbkt->list);
#endif
    } else {
        /* nfree == nblk */
        /* queue or reclaim a free buffer */
        if (tls) {
            if (tbkt->nbuf >= membktnbuftls(type, slot)) {
                memdequeuebuftls(buf, tbkt);
            } else {
                
                return;
            }
            memlkbit(&gbkt->list);
        }
        if ((MEMUNMAP) && gbkt->nbuf >= membktnbufglob(type, slot)) {
            if (!tls) {
                memdequeuebufglob(buf, gbkt);
            }
#if (MEMBKTLOCK)
            memunlkbkt(gbkt);
#else
            memrelbit(&gbkt->list);
#endif
#if (MEMSTAT)
            g_memstat.nbunmap += buf->size;
#endif
            VALGRINDRMPOOL(buf->base);
            unmapanon(buf, buf->size);
            return;
        } else if (tls) {
            /* add buffer in front of global list */
#if (MEMBKTLOCK)
            buf->prev = NULL;
            head = gbkt->list;
#else
            upval = (MEMADR_T)gbkt->list;
            buf->prev = NULL;
            upval &= ~MEMLKBIT;
            head = (struct membuf *)upval;
#endif
            if (head) {
                head->prev = buf;
            }
            buf->next = head;
            buf->tls = NULL;
            gbkt->nbuf++;
//            m_setbit(&buf->info, MEMBUFGLOBBITID);
            /* this will unlock the list (set the low-bit to zero) */
#if (MEMBKTLOCK)
            gbkt->list = buf;
            memunlkbkt(gbkt);
#else
            m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
#endif

            return;
        } else {
            memrelbit(&gbkt->list);

            return;
        }
    }

    return;
}

