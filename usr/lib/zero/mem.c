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
    MEMWORD_T               bufsz;
    MEMWORD_T               nb;
    struct membuf          *buf;
    struct membuf          *head;
    MEMADR_T                upval;

    if (g_memtls) {
        bufsz = g_memtls->nbytetab[MEMSMALLBUF];
        for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
            src = &g_memtls->smallbin[slot];
            dest = &g_mem.deadsmall[slot];
#if 0
            dest = &g_mem.smallbin[slot];
#endif
            head = src->list;
            if (head) {
                buf = head;
#if (MEMBUFRELMAP)
//                membuffreerel(buf);
#endif
                buf->tls = NULL;
                while (buf->next) {
                    buf = buf->next;
#if (MEMBUFRELMAP)
//                    membuffreerel(buf);
#endif
                    buf->tls = NULL;
                }
#if (MEMDEBUGDEADLOCK)
                memlkbitln(dest);
#else
                memlkbit(&dest->list);
#endif
                upval = (MEMADR_T)dest->list;
                upval &= ~MEMLKBIT;
                head = (struct membuf *)upval;
//                buf->prev = NULL;
                buf->next = head;
                if (upval) {
                    head->prev = buf;
                }
#if (MEMDEBUGDEADLOCK)
                dest->line = __LINE__;
#endif
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
            }
        }
        nb = g_mem.nbytetab[MEMSMALLBUF];
        nb += bufsz;
        g_mem.nbytetab[MEMSMALLBUF] = bufsz;
        bufsz = g_memtls->nbytetab[MEMPAGEBUF];
        for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
            src = &g_memtls->pagebin[slot];
            dest = &g_mem.deadpage[slot];
#if 0
            dest = &g_mem.pagebin[slot];
#endif
            head = src->list;
            if (head) {
                buf = head;
#if (MEMBUFRELMAP)
//                membuffreerel(buf);
#endif
                buf->tls = NULL;
                while (buf->next) {
                    buf = buf->next;
#if (MEMBUFRELMAP)
//                    membuffreerel(buf);
#endif
                    buf->tls = NULL;
                }
#if (MEMDEBUGDEADLOCK)
                memlkbitln(dest);
#else
                memlkbit(&dest->list);
#endif
                upval = (MEMADR_T)dest->list;
                upval &= ~MEMLKBIT;
                head = (struct membuf *)upval;
//                buf->prev = NULL;
                buf->next = head;
                if (upval) {
                    head->prev = buf;
                }
#if (MEMDEBUGDEADLOCK)
                dest->line = __LINE__;
#endif
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
            }
        }
        nb = g_mem.nbytetab[MEMSMALLBUF];
        nb += bufsz;
        g_mem.nbytetab[MEMSMALLBUF] = bufsz;
        priolkfin();
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

    pthread_once(&g_initonce, meminit);
#if (MEMDYNTLS)
    tls = mapanon(0, memtlssize());
    if (tls != MAP_FAILED) {
        adr = (struct memtls *)memgentlsadr((MEMUWORD_T *)tls);
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

    return g_memtls;
}

static void
memprefork(void)
{
    MEMWORD_T slot;

    spinlk(&g_mem.initlk);
    memgetlk(&g_mem.heaplk);
#if (MEMMULTITAB)
    for (slot = 0 ; slot < MEMLVL1ITEMS ; slot++) {
        memlkbit(&g_mem.tab[slot].tab);
    }
#elif (!MEMLFHASH)
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
            memlkbit(&g_mem.hash[slot].chain);
        }
    }
#endif
    for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.smallbin[slot]);
#else
        memlkbit(&g_mem.smallbin[slot].list);
#endif
    }
    for (slot = 0 ; slot < MEMBIGSLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.bigbin[slot]);
#else
        memlkbit(&g_mem.bigbin[slot].list);
#endif
    }
    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.pagebin[slot]);
#else
        memlkbit(&g_mem.pagebin[slot].list);
#endif
    }

    return;
}

static void
mempostfork(void)
{
    MEMWORD_T slot;

    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.pagebin[slot]);
#else
        memrelbit(&g_mem.pagebin[slot].list);
#endif
    }
    for (slot = 0 ; slot < MEMBIGSLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.bigbin[slot]);
#else
        memrelbit(&g_mem.bigbin[slot].list);
#endif
    }
    for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.smallbin[slot]);
#else
        memrelbit(&g_mem.smallbin[slot].list);
#endif
    }
#if (MEMMULTITAB)
    for (slot = 0 ; slot < MEMLVL1ITEMS ; slot++) {
        memrelbit(&g_mem.tab[slot].tab);
    }
#elif (!MEMLFHASH)
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
            memrelbit(&g_mem.hash[slot].chain);
        }
    }
#endif
    memrellk(&g_mem.heaplk);
    spinunlk(&g_mem.initlk);

    return;
}

NORETURN
void
memexit(int sig)
{
#if (MEMSTAT) && 0
    memprintstat();
#endif

    exit(sig);
}

NORETURN
void
memquit(int sig)
{
#if (MEMSTAT) && 0
    memprintstat();
#endif
    fprintf(stderr, "CAUGHT signal %d, aborting\n", sig);
    abort();
}

void
meminit(void)
{
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    void       *heap;
    intptr_t    ofs;
#endif
    void       *ptr;

    fprintf(stderr, "MEMHASHARRAYITEMS == %d\n", MEMHASHARRAYITEMS);
    spinlk(&g_mem.initlk);
#if 0
    signal(SIGQUIT, memexit);
    signal(SIGINT, memexit);
    signal(SIGTERM, memexit);
#endif
    signal(SIGSEGV, memquit);
    signal(SIGABRT, memquit);
    pthread_atfork(memprefork, mempostfork, mempostfork);
    pthread_key_create(&g_thrkey, memreltls);
#if (MEMSTAT)
    atexit(memprintstat);
#endif
#if (MEMMULTITAB)
    ptr = mapanon(0, MEMLVL1ITEMS * sizeof(struct memtab));
#elif (MEMNEWHASH)
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhashlist));
#if (MEMSTAT)
    g_memstat.nbhashtab = MEMHASHITEMS * sizeof(struct memhash);
#endif
#elif (MEMHUGELOCK)
    ptr = mapanon(0, MEMLVL1ITEMS * sizeof(struct memtabl0));
#else
    ptr = mapanon(0, MEMLVL1ITEMS * sizeof(struct memtab));
#endif
    if (ptr == MAP_FAILED) {

        crash(ptr != MAP_FAILED);
    }
#if (MEMMULTITAB)
    g_mem.tab = ptr;
#elif (MEMNEWHASH)
    g_mem.hash = ptr;
#endif
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
    spinunlk(&g_mem.initlk);

    return;
}

MEMPTR_T
memcalcadr(struct membuf *buf, MEMPTR_T ptr, MEMWORD_T size, MEMWORD_T align)
//          MEMWORD_T id)
{
    MEMPTR_T   adr = ptr;
    MEMWORD_T  type = memgetbuftype(buf);
    MEMWORD_T  slot;
    MEMWORD_T  bsz;

    if (align <= CLSIZE) {
        slot = memgetbufslot(buf);
        bsz = membufblksize(buf, type, slot);
        ptr = memgenadr(adr, bsz, size);
    } else if ((MEMADR_T)ptr & (align - 1)) {
        ptr = memalignptr(adr, align);
    }
#if 0
    if (type != MEMPAGEBUF) {
        membufsetadr(buf, ptr, adr);
    } else {
        membufsetpageadr(buf, id, adr);
    }
#endif

    return ptr;
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
#endif
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
    buf->info = flg;    // possible MEMHEAPBIT
    memsetbufslot(buf, slot);
    memsetbufnblk(buf, nblk);
    memsetbuftype(buf, MEMSMALLBUF);
    buf->size = bufsz;
#if (MEMTEST)
    _memchkbuf(buf, MEMSMALLBUF, nblk, flg, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitsmallbuf(struct membuf *buf,
                MEMWORD_T size, MEMWORD_T align,
                MEMWORD_T nblk)
{
    MEMPTR_T  adr = (MEMPTR_T)buf;
    MEMPTR_T  ptr = adr + membufblkofs(nblk);

    /* initialise freemap */
    membufinitfree(buf);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
//    ptr = memcalcadr(buf, ptr, size, align, 0);
    ptr = memcalcadr(buf, ptr, size, align);
    memsetbuf(ptr, buf, 0);
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
#if (MEMBITFIELD)
    memclrbufflg(buf, 1);
#else
    buf->info = 0;
#endif
    memsetbufslot(buf, slot);
    memsetbufnblk(buf, nblk);
    memsetbuftype(buf, MEMPAGEBUF);
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
               MEMWORD_T size, MEMWORD_T align,
               MEMWORD_T nblk)
{
    MEMPTR_T adr = (MEMPTR_T)buf;
    MEMPTR_T ptr = adr + membufblkofs(nblk);

    /* initialise freemap */
    membufinitfree(buf);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
//    ptr = memcalcadr(buf, ptr, size, align, 0);
    ptr = memcalcadr(buf, ptr, size, align);
    memsetbuf(ptr, buf, 0);
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
#if (MEMBITFIELD)
    memclrbufflg(buf, 1);
#else
    buf->info = 0;
#endif
    memsetbufslot(buf, slot);
    memsetbufnblk(buf, nblk);
    memsetbuftype(buf, MEMBIGBUF);
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
              MEMWORD_T size, MEMWORD_T align,
              MEMWORD_T nblk)
{
    MEMPTR_T adr = (MEMPTR_T)buf;
    MEMPTR_T ptr = adr + membufblkofs(nblk);

    membufinitfree(buf);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
//    ptr = memcalcadr(buf, ptr, size, align, 0);
    ptr = memcalcadr(buf, ptr, size, align);
    memsetbuf(ptr, buf, 0);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

#if (MEMMULTITAB)

MEMPTR_T
memsetbuf(MEMPTR_T ptr, struct membuf *buf)
{
    MEMADR_T        val = (MEMADR_T)buf;
    struct memtab  *itab;
    struct memtab  *tab;
    struct memitem *item;
    long             k1;
    long             k2;
    long             k3;
    long             k4;
    void            *pstk[2] = { NULL };
    
    memgetkeybits(ptr, k1, k2, k3, k4);
//    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (!itab) {
        itab = mapanon(0, MEMLVLITEMS * sizeof(struct memtab));
        if (itab == MAP_FAILED) {
//            memrellk(&g_mem.tab[k1].lk);
            
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
//            memrellk(&g_mem.tab[k1].lk);

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
//            memrellk(&g_mem.tab[k1].lk);
            
            return NULL;
        }
        tab->tab = itab;
    }
    item = (struct memitem *)&itab[k4];
//    item->nref++;
    item->val = val;
//    memrellk(&g_mem.tab[k1].lk);
    
    return ptr;
}

struct membuf *
memfindbuf(void *ptr, long rel)
{
    struct membuf  *buf = NULL;
    struct memtab  *itab;
    struct memtab  *tab;
    struct memitem *item;
    long            k1;
    long            k2;
    long            k3;
    long            k4;

    memgetkeybits(ptr, k1, k2, k3, k4);
//    memgetlk(&g_mem.tab[k1].lk);
    itab = g_mem.tab[k1].tab;
    if (itab) {
        tab = &itab[k2];
        itab = tab->tab;
        if (itab) {
            tab = &itab[k3];
            itab = tab->tab;
            if (itab) {
                item = (struct memitem *)&itab[k4];
                buf = item->val;
                if (rel) {
                    memrelblk(ptr, buf);
#if 0
                    if (!--item->nref) {
                        item->val = 0;
                    }
#endif
                }
            }
        }
    }
//    memrellk(&g_mem.tab[k1].lk);

    return buf;
}

#elif (MEMNEWHASH)

#if (MEMNEWHASHTAB)
static void
meminithashitem(MEMPTR_T data)
{
    struct memhash *item = (struct memhash *)data;
    MEMUWORD_T     *uptr;

    data += offsetof(struct memhash, data);
    item->chain = NULL;
    uptr = (MEMUWORD_T *)data;
    item->ntab = 0;
    item->tab = (struct memhashitem *)uptr;
    item->list = NULL;

    return;
}
#else
static void
meminithashitem(MEMPTR_T data)
{
    struct memhash *item = (struct memhash *)data;
    MEMUWORD_T     *uptr;

    data += offsetof(struct memhash, data);
    item->chain = NULL;
    uptr = (MEMUWORD_T *)data;
    uptr = memgenhashtabadr(uptr);
    item->ntab = 0;
    item->tab = (struct memhashitem *)uptr;
    item->list = NULL;

    return;
}
#endif

#endif

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

#if (!MEMLFHASH)
    memlkbit(&g_mem.hashbuf);
#endif
    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    if (upval) {
        item = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item->chain);
//        meminithashitem(item);
    } else {
        n = 4 * PAGESIZE / memhashsize();
        bsz = 4 * PAGESIZE;
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

#if defined(MEMHASHNREF) && (MEMHASHNREF)
static void
membufhashitem(struct memhash *item)
{
    MEMADR_T upval;

    memlkbit(&g_mem.hashbuf);
    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    item->chain = (struct memhash *)upval;
    m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item);

    return;
}
#endif

MEMADR_T
membufop(MEMPTR_T ptr, MEMWORD_T op, struct membuf *buf, MEMWORD_T id)
{
    MEMPTR_T                adr = ptr;
    MEMADR_T                page = (MEMADR_T)adr >> PAGESIZELOG2;
    MEMUWORD_T              key = memhashptr(page) & (MEMHASHITEMS - 1);
    MEMADR_T                desc;
    MEMADR_T                upval;
    MEMADR_T                val;
    struct memhash         *blk;
    struct memhash         *prev;
    struct memhashitem     *slot;
    struct memhashitem     *src;
//    volatile struct memtls *tls;
    MEMWORD_T               type;
    MEMWORD_T               lim;
    MEMUWORD_T              n;
    MEMWORD_T               found;
#if (MEMBFHASH)
    struct memhashitem     *item;
    MEMADRDIFF_T            mask;
#endif

//    fprintf(stderr, "locking hash chain %lx\n", key);
#if (!MEMLFHASH)
    memlkbit(&g_mem.hash[key].chain);
#endif
    upval = (MEMADR_T)g_mem.hash[key].chain;
#if (!MEMLFHASH)
    upval &= ~MEMLKBIT;
#endif
    found = 0;
    prev = NULL;
    blk = (struct memhash *)upval;
    desc = 0;
    slot = NULL;
    while ((blk) && !found) {
        lim = blk->ntab;
        src = blk->tab;
        prev = NULL;
//            slot = &src[7];
#if (MEMBFHASH)
        do {
            n = min(lim, 8);
            switch (n) {
                /*
                 * if found, the mask will be -1 (all 1-bits), and val will be
                 * the item address
                 * if not found, the mask will be 0 and so will val/slot
                 */
                
                case 8:
                    item = &src[7];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 7:
                    item = &src[6];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                    if (slot) {

                        break;
                    }
                case 6:
                    item = &src[5];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 5:
                    item = &src[4];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                    if (slot) {

                        break;
                    }
                case 4:
                    item = &src[3];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 3:
                    item = &src[2];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                    if (slot) {

                        break;
                    }
                case 2:
                    item = &src[1];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 1:
                    item = &src[0];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 0:
                default:
                    
                    break;
            }
            lim -= n;
            src += n;
        } while (!slot && !found);
        if (!slot) {
            prev = blk;
            blk = blk->chain;
        } else {
            found++;
            desc = slot->val;
        }
#else
        do {
            n = min(lim, 8);
//            slot = &src[7];
            switch (n) {
                case 8:
                    if (src[7].adr == page) {
                        slot = &src[7];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 7:
                    if (src[6].adr == page) {
                        slot = &src[6];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 6:
                    if (src[5].adr == page) {
                        slot = &src[5];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 5:
                    if (src[4].adr == page) {
                        slot = &src[4];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 4:
                    if (src[3].adr == page) {
                        slot = &src[3];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 3:
                    if (src[2].adr == page) {
                        slot = &src[2];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 2:
                    if (src[1].adr == page) {
                        slot = &src[1];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 1:
                    if (src[0].adr == page) {
                        slot = &src[0];
                        found++;
                        desc = slot->val;
                        
                        break;
                    }
                case 0:
                default:
                    
                    break;
            }
            lim -= n;
            src += n;
        } while ((lim) && !found);
        if (!found) {
            prev = blk;
            blk = blk->chain;
        }
#endif
    }
//    upval = (MEMADR_T)g_mem.hash[key].chain;
    if (!found) {
        if (op == MEMHASHDEL || op == MEMHASHCHK) {
#if (!MEMLFHASH)
            memrelbit(&g_mem.hash[key].chain);
#endif

            return MEMHASHNOTFOUND;
        } else {
            desc = (MEMADR_T)buf;
//            upval &= ~MEMLKBIT;
            blk = (struct memhash *)upval;
            if (id >= 0) {
                desc |= id;
            }
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
                        crash(slot != NULL);
#endif
                        if (prev) {
                            prev->chain = blk->chain;
                            blk->chain = (struct memhash *)upval;
#if (MEMHASHLOCK)
                            g_mem.hash[key].chain = blk;
#elif (MEMNEWHASH)
                            m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                        (m_atomic_t)blk);
#endif
                        } else {
#if (!MEMLFHASH)
                            memrelbit(&g_mem.hash[key].chain);
#endif
                        }

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
            blk->list = &g_mem.hash[key];
            slot->adr = page;
            slot->val = desc;
#if (MEMHASHLOCK)
            g_mem.hash[key].chain = blk;
            memrellk(&g_mem.hash[key].lk);
#else
            m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                        (m_atomic_t)blk);
#endif
#if (MEMDEBUG)
            crash(desc != 0);
#endif
            
            return desc;
        }
    } else {
//    upval &= ~MEMLKBIT;
#if defined(MEMHASHNACT) && (MEMHASHNACT)
        slot->nact++;
#endif
        if (op == MEMHASHDEL) {
            id = desc & MEMPAGEINFOMASK;
            desc &= ~MEMPAGEINFOMASK;
            buf = (struct membuf *)desc;
//            tls = buf->tls;
            type = memgetbuftype(buf);
            VALGRINDPOOLFREE(buf->base, ptr);
            if (type != MEMPAGEBUF) {
//                adr = membufgetadr(buf, ptr);
                id = membufblkid(buf, ptr);
            }
#if 0
            if (tls && tls != g_memtls) {
                membufsetrel(buf, id);
            } else {
                memrelblk(buf, id);
            }
#endif
            memrelblk(buf, id);
#if defined(MEMHASHNREF) && (MEMHASHNREF)
            slot->nref--;
            n = blk->ntab;
            if (!slot->nref) {
                if (n == 1) {
                    if (prev) {
                        prev->chain = blk->chain;
#if (!MEMLFHASH)
                        memrelbit(&g_mem.hash[key].chain);
#endif
                    } else {
                        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                    (m_atomic_t)blk->chain);
                    }
                    membufhashitem(blk);
                } else {
                    n--;
                    src = &blk->tab[n];
                    slot->nref = src->nref;
#if defined(MEMHASHNACT) && (MEMHASHNACT)
                    slot->nact = src->nact;
#endif
//                upval &= ~MEMLKBIT;
                    slot->adr = src->adr;
                    slot->val = src->val;
                    src->adr = 0;
                    src->val = 0;
                    blk->ntab = n;
                }
            }
#endif /* MEMHASHNREF */
#if (MEMDEBUG)
            crash(desc != 0);
#endif
#if (!MEMLFHASH)
            memrelbit(&g_mem.hash[key].chain);
#endif

            return desc;
#if defined(MEMHASHNREF) && (MEMHASHNREF)
        } else if (op == MEMHASHADD) {
            slot->nref++;
#endif
        }
//    upval &= ~MEMLKBIT;
        if (prev) {
            prev->chain = blk->chain;
            blk->chain = (struct memhash *)upval;
            m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                        (m_atomic_t)blk);
#if (!MEMLFHASH)
        } else {
            memrelbit(&g_mem.hash[key].chain);
#endif
        }
    }
#if (MEMDEBUG)
    crash(desc != 0);
#endif
    
    return desc;
}

MEMPTR_T
memsetbuf(MEMPTR_T ptr, struct membuf *buf, MEMWORD_T id)
{
    MEMADR_T desc = membufop(ptr, MEMHASHADD, buf, id);

    return (MEMPTR_T)desc;
}

MEMPTR_T
memgetblktls(MEMWORD_T type, MEMWORD_T slot, MEMWORD_T size, MEMWORD_T align)
{
    volatile struct membkt *bkt;
    struct membuf          *head;
    MEMPTR_T                ptr;
    MEMPTR_T                adr;
    MEMWORD_T               blksz;
    MEMWORD_T               nfree;
#if (MEMDEBUG)
    MEMWORD_T               nblk;
#endif
    MEMWORD_T               id;

    if (type == MEMSMALLBUF) {
        blksz = MEMWORD(1) << slot;
        bkt = &g_memtls->smallbin[slot];
    } else {
        blksz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_memtls->pagebin[slot];
    }
    do {
        head = bkt->list;
        if (!head) {
            
            return NULL;
        }
        nfree = memgetbufnfree(head);
#if (MEMBUFRELMAP) && 0
        if (!nfree) {
            membuffreerel(head);
            nfree = memgetbufnfree(head);
        }
#endif
        if (!nfree) {
        /* head shall be disconnected from all lists */
            if (head->next) {
                head->next->prev = NULL;
            }
#if (MEMDEBUGDEADLOCK)
            bkt->line = __LINE__;
#endif
            bkt->list = head->next;
            head->tls = NULL;
            head->prev = NULL;
            head->next = NULL;
        } else {
            id = membufgetfree(head);
#if (MEMDEBUG)
            nblk = memgetbufnblk(head);
            crash(nfree > 0 && nfree <= nblk);
#endif
            if (type != MEMPAGEBUF) {
                adr = membufblkadr(head, id);
            } else {
                adr = membufpageadr(head, id);
            }
            nfree--;
//            ptr = memcalcadr(head, adr, size, align, id);
            ptr = memcalcadr(head, adr, size, align);
            if (type != MEMPAGEBUF) {
                memsetbuf(ptr, head, -1);
            } else {
                memsetbuf(ptr, head, id);
            }
            memsetbufnfree(head, nfree);
            bkt->bufsz -= blksz;
            VALGRINDPOOLALLOC(head->base, ptr, size);
            if (!nfree) {
                /* head shall be disconnected from all lists */
                if (head->next) {
                    head->next->prev = NULL;
                }
#if (MEMDEBUGDEADLOCK)
                bkt->line = __LINE__;
#endif
                bkt->list = head->next;
                head->tls = NULL;
                head->prev = NULL;
                head->next = NULL;
            }

            return ptr;
        }
    } while (1);

    /* NOTREACHED */
    return NULL;
}

MEMPTR_T
memgetblkdead(MEMWORD_T type, MEMWORD_T slot, MEMWORD_T size, MEMWORD_T align)
{
    volatile struct membkt *bkt;
    struct membuf          *head;
    MEMPTR_T                ptr;
    MEMPTR_T                adr;
    MEMWORD_T               blksz;
    MEMWORD_T               nfree;
#if (MEMDEBUG)
    MEMWORD_T               nblk;
#endif
    MEMWORD_T               id;
    MEMADR_T                upval;

    if (type == MEMSMALLBUF) {
        blksz = MEMWORD(1) << slot;
        bkt = &g_mem.deadsmall[slot];
    } else {
        blksz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_mem.deadpage[slot];
    }
#if (MEMDEBUGDEADLOCK)
    memlkbitln(bkt);
#else
    memlkbit(&bkt->list);
#endif
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    head = (struct membuf *)upval;
    if (!head) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(bkt);
#else
        memrelbit(&bkt->list);
#endif
        
        return NULL;
    }
#if (MEMBUFRELMAP)
    membuffreerel(head);
#endif
    nfree = memgetbufnfree(head);
    if (!nfree) {
        /* head shall be disconnected from all lists */
        if (head->next) {
            head->next->prev = NULL;
        }
#if (MEMDEBUGDEADLOCK)
        bkt->line = __LINE__;
#endif
        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)head->next);
        head->tls = NULL;
        head->prev = NULL;
        head->next = NULL;
    } else {
        id = membufgetfree(head);
#if (MEMDEBUG)
        nblk = memgetbufnblk(head);
        crash(nfree > 0 && nfree <= nblk);
#endif
        if (type != MEMPAGEBUF) {
            adr = membufblkadr(head, id);
        } else {
            adr = membufpageadr(head, id);
        }
        nfree--;
//            ptr = memcalcadr(head, adr, size, align, id);
        ptr = memcalcadr(head, adr, size, align);
        if (type != MEMPAGEBUF) {
            memsetbuf(ptr, head, -1);
        } else {
            memsetbuf(ptr, head, id);
        }
        memsetbufnfree(head, nfree);
        bkt->bufsz -= blksz;
        VALGRINDPOOLALLOC(head->base, ptr, size);
        if (!nfree) {
            /* head shall be disconnected from all lists */
            if (head->next) {
                head->next->prev = NULL;
            }
#if (MEMDEBUGDEADLOCK)
            bkt->line = __LINE__;
#endif
            m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)head->next);
            head->tls = NULL;
            head->prev = NULL;
            head->next = NULL;
        }
    }        

    return ptr;
}

MEMPTR_T
memgetblkglob(MEMWORD_T type, MEMWORD_T slot, MEMWORD_T size, MEMWORD_T align)
{
    volatile struct membkt *bkt;
    struct membuf          *head;
    MEMPTR_T                ptr;
    MEMPTR_T                adr;
    MEMWORD_T               blksz;
    MEMWORD_T               nfree;
#if (MEMDEBUG)
    MEMWORD_T               nblk;
#endif
    MEMWORD_T               id;
    MEMADR_T                upval;

    if (type == MEMSMALLBUF) {
        blksz = MEMWORD(1) << slot;
        bkt = &g_mem.smallbin[slot];
    } else if (type == MEMPAGEBUF) {
        blksz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_mem.pagebin[slot];
    } else {
        blksz = MEMWORD(1) << slot;
        bkt = &g_mem.bigbin[slot];
    }
#if (MEMDEBUGDEADLOCK)
    memlkbitln(bkt);
#else
    memlkbit(&bkt->list);
#endif
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    head = (struct membuf *)upval;
    if (!head) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(bkt);
#else
        memrelbit(&bkt->list);
#endif

        return NULL;
    }
    nfree = memgetbufnfree(head);
    id = membufgetfree(head);
#if (MEMDEBUG)
    nblk = memgetbufnblk(head);
    crash(nfree > 0 && nfree <= nblk);
#endif
    nfree--;
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(head, id);
    } else {
        adr = membufpageadr(head, id);
    }
    memsetbufnfree(head, nfree);
//    ptr = memcalcadr(head, adr, size, align, id);
    ptr = memcalcadr(head, adr, size, align);
    memsetbuf(ptr, head, id);
#if (MEMTEST) && 0
    _memchkptr(head, ptr);
#endif
    bkt->bufsz -= blksz;
    VALGRINDPOOLALLOC(head->base, ptr, size);
    if (!nfree) {
        /* head shall be disconnected from all lists */
        if (head->next) {
            head->next->prev = NULL;
        }
#if (MEMDEBUGDEADLOCK)
        bkt->line = __LINE__;
#endif
        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)head->next);
        m_clrbit((m_atomic_t *)&head->info, MEMBUFGLOBBITID);
        head->prev = NULL;
        head->next = NULL;
    } else {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(bkt);
#else
        memrelbit(&bkt->list);
#endif
    }

    return ptr;
}

MEMPTR_T
memgetblk(MEMWORD_T slot, MEMWORD_T type, MEMWORD_T size, MEMWORD_T align)
{
    MEMPTR_T                ptr;
    MEMWORD_T               nblk;
    MEMWORD_T               flg;
    struct membuf          *buf;
    struct membuf          *head;
    MEMWORD_T               bktsz;
    MEMADR_T                upval;
    volatile struct membkt *dest;
    volatile struct memtls *tls;
    
    ptr = NULL;
    if (type != MEMBIGBUF) {
        ptr = memgetblktls(type, slot, size, align);
        if (!ptr) {
            ptr = memgetblkdead(type, slot, size, align);
        }
    }
    if (!ptr) {
        ptr = memgetblkglob(type, slot, size, align);
    }
    if (!ptr) {
        nblk = memgetnbufblk(type, slot);
        if (type == MEMSMALLBUF) {
            bktsz = nblk << slot;
            buf = memallocsmallbuf(slot, nblk);
            if (buf) {
                ptr = meminitsmallbuf(buf, size, align, nblk);
            }
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
        } else if (type == MEMPAGEBUF) {
            bktsz = PAGESIZE + (PAGESIZE << slot);
            buf = memallocpagebuf(slot, nblk);
            if (buf) {
                ptr = meminitpagebuf(buf, size, align, nblk);
            }
        } else {
            bktsz = nblk << slot;
            buf = memallocbigbuf(slot, nblk);
            if (buf) {
                ptr = meminitbigbuf(buf, size, align, nblk);
            }
        }
        if (ptr && nblk > 1) {
            if (type != MEMBIGBUF) {
                tls = g_memtls;
                if (type == MEMSMALLBUF) {
                    dest = &tls->smallbin[slot];
                } else {
                    dest = &tls->pagebin[slot];
                }
                head = dest->list;
            } else {
                dest = &g_mem.bigbin[slot];
#if (MEMDEBUGDEADLOCK)
                memlkbitln(dest);
#else
                memlkbit(&dest->list);
#endif
                upval = (MEMADR_T)dest->list;
                upval &= ~MEMLKBIT;
                head = (struct membuf *)upval;
            }
            buf->prev = NULL;
            if (head) {
                head->prev = buf;
            }
            buf->next = head;
            dest->bufsz += bktsz;
            if (type != MEMBIGBUF) {
                buf->tls = tls;
                dest->list = buf;
            } else {
                m_setbit(&buf->info, MEMBUFGLOBBITID);
                m_syncwrite((m_atomic_t *)&dest->list,
                            (m_atomic_t)buf);
            }
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
    src->list = head;

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
#if (MEMDEBUGDEADLOCK)
    src->line = __LINE__;
#endif
    m_syncwrite((m_atomic_t *)&src->list, (m_atomic_t)head);

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
    MEMWORD_T               bufsz;
    MEMWORD_T               bktsz;
    MEMWORD_T               lim;
    MEMADR_T                upval;
    MEMWORD_T               isglob;
    MEMWORD_T               val;

    tls = buf->tls;
#if 0
    if (tls && tls != g_memtls) {
        membufsetrel(buf, id);

        return;
    }
#endif
    isglob = 0;
    if (buf->info & MEMBUFGLOBBITID) {
        isglob = 1;
    }
    type = memgetbuftype(buf);
    slot = memgetbufslot(buf);
    if (type == MEMSMALLBUF) {
        gbkt = &g_mem.smallbin[slot];
    } else if (type == MEMPAGEBUF) {
        gbkt = &g_mem.pagebin[slot];
    } else {
        gbkt = &g_mem.bigbin[slot];
    }
    if (isglob) {
#if (MEMDEBUGDEADLOCK)
        memlkbitln(gbkt);
#else
        memlkbit(&gbkt->list);
#endif
#if (MEMBUFRELMAP)
    } else if ((tls) && tls != g_memtls) {
        membufsetrel(buf, id);

        return;
#endif
    }
    nfree = memgetbufnfree(buf);
    nblk = memgetbufnblk(buf);
    nfree++;
#if (MEMTEST)
    _memchkptr(buf, adr);
#endif
    setbit(buf->freemap, id);
    memsetbufnfree(buf, nfree);
    if (nfree != 1 && nfree != nblk) {
        /* buffer not totally free or allocated */
        if (isglob) {
#if (MEMDEBUGDEADLOCK)
            memrelbitln(gbkt);
#else
            memrelbit(&gbkt->list);
#endif
        }

        return;
    }
#if (MEMDEBUG)
    crash(nfree <= nblk);
#endif
    if (nfree == 1) {
        if (!isglob) {
#if (MEMDEBUGDEADLOCK)
            memlkbitln(gbkt);
#else
            memlkbit(&gbkt->list);
#endif
        }
        /* add buffer in front of global list */
        upval = (MEMADR_T)gbkt->list;
        buf->prev = NULL;
        upval &= ~MEMLKBIT;
        head = (struct membuf *)upval;
        if (head) {
            head->prev = buf;
        }
        buf->next = head;
#if (MEMDEBUGDEADLOCK)
        gbkt->line = __LINE__;
#endif
        m_setbit(&buf->info, MEMBUFGLOBBITID);
        /* this will unlock the list (set the low-bit to zero) */
        m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
        
        return;
    } else if (nfree == nblk) {
        /* queue or reclaim a free buffer */
        bufsz = 0;
        val = 0;
        if (type != MEMPAGEBUF) {
            bufsz = nblk << slot;
        } else {
            bufsz = PAGESIZE + PAGESIZE * slot;
        }
        if (!isglob) {
            if (!tls && tls != g_memtls) {

                return;
            }
            tbkt = NULL;
            if (type == MEMSMALLBUF) {
                tbkt = &tls->smallbin[slot];
                bktsz = tls->nbytetab[MEMSMALLBUF];
                lim = MEMSMALLTLSLIM;
                bufsz = nblk << slot;
                bktsz += bufsz;
                if (bktsz > lim) {
                    memdequeuebuftls(buf, tbkt);
                } else {

                    return;
                }
            } else if (type == MEMPAGEBUF) {
                tbkt = &tls->pagebin[slot];
                bktsz = tls->nbytetab[MEMPAGEBUF];
                lim = MEMPAGETLSLIM;
                bufsz = PAGESIZE + PAGESIZE * slot;
                bktsz += bufsz;
                if (bktsz > lim) {
                    memdequeuebuftls(buf, tbkt);
                } else {

                    return;
                }
            }
        }
        if (type == MEMSMALLBUF) {
            val = 1;
            bktsz = g_mem.nbytetab[MEMSMALLBUF];
            if (!isglob) {
                bktsz += bufsz;
            }
        } else {
            if (type == MEMPAGEBUF) {
                bktsz = g_mem.nbytetab[MEMPAGEBUF];
                lim = MEMPAGEGLOBLIM;
            } else {
                bktsz = g_mem.nbytetab[MEMBIGBUF];
                lim = MEMBIGGLOBLIM;
            }
            if (!isglob) {
                bktsz += bufsz;
            }
            val = (bktsz <= lim);
        }
        if (val) {
            if (isglob) {
#if (MEMDEBUGDEADLOCK)
                memrelbitln(gbkt);
#else
                memrelbit(&gbkt->list);
#endif
                
                return;
            }
#if (MEMDEBUGDEADLOCK)
            memlkbitln(gbkt);
#else
            memlkbit(&gbkt->list);
#endif
            /* add buffer in front of global list */
            upval = (MEMADR_T)gbkt->list;
            buf->prev = NULL;
            upval &= ~MEMLKBIT;
            head = (struct membuf *)upval;
            if (upval) {
                head->prev = buf;
            }
            buf->next = head;
            g_mem.nbytetab[type] = bktsz;
#if (MEMDEBUGDEADLOCK)
            gbkt->line = __LINE__;
#endif
            m_setbit(&buf->info, MEMBUFGLOBBITID);
            /* this will unlock the list (set the low-bit to zero) */
            m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
        } else {
            bktsz = g_mem.nbytetab[type];
            if (isglob) {
                /* unqueue from global list */
                memdequeuebufglob(buf, gbkt);
#if (MEMDEBUGDEADLOCK)
                memrelbitln(gbkt);
#else
                memrelbit(&gbkt->list);
#endif
            }
#if (MEMSTAT)
            g_mem.nbytetab[type] -= bufsz;
#endif
            /* unmap the buffer */
#if (MEMSTAT)
            g_memstat.nbunmap += buf->size;
#endif
            VALGRINDRMPOOL(buf->base);
            unmapanon(buf, buf->size);
        }
    }

    return;
}

