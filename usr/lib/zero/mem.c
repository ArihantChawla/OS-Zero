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

static pthread_once_t               g_initonce = PTHREAD_ONCE_INIT;
static pthread_key_t                g_thrkey;
THREADLOCAL MEMUWORD_T              g_memtlsinit;
THREADLOCAL volatile struct memtls *g_memtls;
struct mem                          g_mem;
#if (MEMSTAT)
struct memstat                      g_memstat;
#endif

static void
memfreetls(void *arg)
{
    void                   *adr = arg;
    volatile struct membkt *src;
    volatile struct membkt *dest;
    MEMUWORD_T              slot;
    MEMUWORD_T              n;
    struct membuf          *head;
    struct membuf          *buf;
    MEMADR_T                upval;

    if (g_memtls) {
        for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
            src = &g_memtls->smallbin[slot];
            dest = &g_mem.smallbin[slot];
            head = src->list;
            if (head) {
                buf = head;
                buf->bkt = dest;
                while (buf->next) {
                    buf = buf->next;
                    buf->bkt = dest;
                }
                n = src->nbuf;
#if (MEMDEBUGDEADLOCK)
                memlkbitln(dest);
#else
                memlkbit(&dest->list);
#endif
                upval = (MEMADR_T)dest->list;
                dest->nbuf += n;
                upval &= ~MEMLKBIT;
//                buf->prev = NULL;
                buf->next = (struct membuf *)upval;
                if (upval) {
                    ((struct membuf *)upval)->prev = buf;
                }
#if (MEMDEBUGDEADLOCK)
                dest->line = __LINE__;
#endif
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
            }
        }
        for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
            src = &g_memtls->pagebin[slot];
            dest = &g_mem.pagebin[slot];
            head = src->list;
            if (head) {
                buf = head;
                buf->bkt = dest;
                while (buf->next) {
                    buf = buf->next;
                    buf->bkt = dest;
                }
                n = src->nbuf;
#if (MEMDEBUGDEADLOCK)
                memlkbitln(dest);
#else
                memlkbit(&dest->list);
#endif
                upval = (MEMADR_T)dest->list;
                dest->nbuf += n;
                upval &= ~MEMLKBIT;
//                buf->prev = NULL;
                buf->next = (struct membuf *)upval;
                if (upval) {
                    ((struct membuf *)upval)->prev = buf;
                }
#if (MEMDEBUGDEADLOCK)
                dest->line = __LINE__;
#endif
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
            }
        }
        unmapanon(adr, memtlssize());
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

struct memtls *
meminittls(void)
{
    struct memtls *tls = NULL;
    struct memtls *adr;
#if (MEM_LK_TYPE == MEM_LK_PRIO)
    unsigned long  val;
#endif

    pthread_once(&g_initonce, meminit);
    tls = mapanon(0, memtlssize());
    if (tls != MAP_FAILED) {
//        adr = (struct memtls *)memgentlsadr((MEMUWORD_T *)tls);
        adr = tls;
#if (MEM_LK_TYPE == MEM_LK_PRIO)
        val = memgetprioval();
        priolkinit(&adr->priolkdata, val);
#endif
        pthread_setspecific(g_thrkey, tls);
        g_memtls = adr;
        g_memtlsinit = 1;
    }

    return tls;
}

static void
memprefork(void)
{
    MEMWORD_T slot;

    spinlk(&g_mem.initlk);
    memgetlk(&g_mem.heaplk);
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
#if (MEMMULTITAB)
    for (slot = 0 ; slot < MEMLVL1ITEMS ; slot++) {
        memlkbit(&g_mem.tab[slot].tab);
    }
#elif (!MEMLFHASH)
    for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
        memlkbit(&g_mem.hash[slot].chain);
    }
#endif

    return;
}

static void
mempostfork(void)
{
    MEMWORD_T slot;

#if (MEMMULTITAB)
    for (slot = 0 ; slot < MEMLVL1ITEMS ; slot++) {
        memrelbit(&g_mem.tab[slot].tab);
    }
#elif (!MEMLFHASH)
    for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
        memrelbit(&g_mem.hash[slot].chain);
    }
#endif
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
    memrellk(&g_mem.heaplk);
    spinunlk(&g_mem.initlk);

    return;
}

void
memexit(int sig)
{
#if (MEMSTAT) && 0
    memprintstat();
#endif

    exit(sig);
}

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

    spinlk(&g_mem.initlk);
    signal(SIGQUIT, memexit);
    signal(SIGINT, memexit);
#if 0
    signal(SIGSEGV, memquit);
    signal(SIGABRT, memquit);
    signal(SIGTERM, memquit);
#endif
    pthread_atfork(memprefork, mempostfork, mempostfork);
    pthread_key_create(&g_thrkey, memfreetls);
#if (MEMSTAT)
    atexit(memprintstat);
#endif
#if (MEMMULTITAB)
    ptr = mapanon(0, MEMLVL1ITEMS * sizeof(struct memtab));
#elif (MEMARRAYHASH) || (MEMNEWHASH)
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhashlist));
#elif (MEMHASH)
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhash));
#if (MEMSTAT)
    g_memstat.nbhash += MEMHASHITEMS * sizeof(struct memhash);
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
#elif (MEMHASH) || (MEMARRAYHASH) || (MEMNEWHASH)
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
    spinunlk(&g_mem.initlk);

    return;
}

MEMPTR_T
memsetadr(struct membuf *buf, MEMPTR_T ptr, MEMUWORD_T size, MEMUWORD_T align,
          MEMWORD_T id)
{
    MEMPTR_T   adr = ptr;
    MEMWORD_T  type = memgetbuftype(buf);
    MEMWORD_T  slot;
    MEMUWORD_T bsz;

    if (align <= CLSIZE) {
        slot = memgetbufslot(buf);
        bsz = membufblksize(buf, type, slot);
        ptr = memgenadr(adr, bsz, size);
    } else if ((MEMADR_T)ptr & (align - 1)) {
        ptr = memalignptr(adr, align);
    }
    if (type != MEMPAGEBUF) {
        membufsetadr(buf, ptr, adr);
    } else {
        membufsetpageadr(buf, id, adr);
    }

    return ptr;
}

static struct membuf *
memallocsmallbuf(MEMWORD_T slot, MEMWORD_T nblk)
{
    MEMPTR_T       adr = SBRK_FAILED;
    MEMWORD_T      bufsz = memsmallbufsize(slot, nblk);
    MEMUWORD_T     flg = 0;
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
#if (MEMBITFIELD)
    memsetbufflg(buf, 1);
#else
    buf->info = flg;    // possible MEMHEAPBIT
#endif
    memsetbufslot(buf, slot);
    memsetbufnblk(buf, nblk);
    memsetbuftype(buf, MEMSMALLBUF);
    buf->size = bufsz;
#if (MEMTEST)
    _memchkbuf(buf, slot, MEMSMALLBUF, nblk, flg, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitsmallbuf(struct membuf *buf,
                MEMUWORD_T size, MEMUWORD_T align,
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
    ptr = memsetadr(buf, ptr, size, align, 0);
    memsetbuf(ptr, buf, 0);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

static struct membuf *
memallocpagebuf(MEMWORD_T slot, MEMWORD_T nblk)
{
    MEMUWORD_T     mapsz = mempagebufsize(slot, nblk);
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
    _memchkbuf(buf, slot, MEMPAGEBUF, nblk, 0, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitpagebuf(struct membuf *buf,
               MEMUWORD_T size, MEMUWORD_T align,
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
    ptr = memsetadr(buf, ptr, size, align, 0);
    memsetbuf(ptr, buf, 0);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

static struct membuf *
memallocbigbuf(MEMWORD_T slot, MEMWORD_T nblk)
{
    MEMUWORD_T     mapsz = membigbufsize(slot, nblk);
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
    _memchkbuf(buf, slot, MEMBIGBUF, nblk, 0, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitbigbuf(struct membuf *buf,
              MEMUWORD_T size, MEMUWORD_T align,
              MEMWORD_T nblk)
{
    MEMPTR_T adr = (MEMPTR_T)buf;
    MEMPTR_T ptr = adr + membufblkofs(nblk);

    membufinitfree(buf);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
    ptr = memsetadr(buf, ptr, size, align, 0);
    memsetbuf(ptr, buf, 0);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

void *
memgetblktls(struct membuf *head, volatile struct membkt *tbkt,
             MEMUWORD_T size, MEMUWORD_T align)
{
    void       *ptr = NULL;
    MEMPTR_T    adr;
    MEMWORD_T   nfree = memgetbufnfree(head);
    MEMUWORD_T  type = memgetbuftype(head);
    MEMWORD_T   id = membufgetfree(head);

#if (MEMDEBUG)
    crash(nfree != 0);
#endif
    nfree--;
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(head, id);
    } else {
        adr = membufpageadr(head, id);
    }
    memsetbufnfree(head, nfree);
    ptr = memsetadr(head, adr, size, align, id);
    memsetbuf(ptr, head, id);
#if (MEMTEST)
    _memchkptr(head, ptr);
#endif
    VALGRINDPOOLALLOC(head->base, ptr, size);
    if (!nfree) {
        /* head shall be disconnected from all lists */
        if (head->next) {
            head->next->prev = NULL;
        }
        tbkt->nbuf--;
#if (MEMDEBUGDEADLOCK)
        tbkt->line = __LINE__;
#endif
        tbkt->list = head->next;
        head->bkt = NULL;
        head->prev = NULL;
        head->next = NULL;
    }

    return ptr;
}

/* NOTE: caller holds a lock on the global bin; we unlock it, though */
void *
memgetblkglob(struct membuf *head, volatile struct membkt *gbkt,
              MEMUWORD_T size, MEMUWORD_T align, volatile struct membkt *tbkt)
{
    void       *ptr = NULL;
    MEMPTR_T    adr;
    MEMWORD_T   nfree = memgetbufnfree(head);
    MEMUWORD_T  type = memgetbuftype(head);
    MEMWORD_T   id = membufgetfree(head);

#if (MEMDEBUG)
    crash(nfree != 0);
#endif
    nfree--;
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(head, id);
    } else {
        adr = membufpageadr(head, id);
    }
    memsetbufnfree(head, nfree);
    ptr = memsetadr(head, adr, size, align, id);
    memsetbuf(ptr, head, id);
#if (MEMTEST)
    _memchkptr(head, ptr);
#endif
    VALGRINDPOOLALLOC(head->base, ptr, size);
    if ((tbkt) || !nfree) {
        if (head->next) {
            head->next->prev = NULL;
        }
        gbkt->nbuf--;
#if (MEMDEBUGDEADLOCK)
        gbkt->line = __LINE__;
#endif
        m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t)head->next);
        if (!nfree) {
            head->bkt = NULL;
            head->prev = NULL;
            head->next = NULL;
        } else if (tbkt) {
            head->next = tbkt->list;
            if (head->next) {
                head->next->prev = head;
            }
            tbkt->list = head;
        }
    } else {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(gbkt);
#else
        memrelbit(&gbkt->list);
#endif
    }

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

static struct memhash *
memgethashitem(void)
{
    struct memhash *item = NULL;
    MEMPTR_T        first;
    struct memhash *prev;
    struct memhash *cur;
    MEMPTR_T        next;
    MEMUWORD_T      bsz;
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
        n = 4 * PAGESIZE / memhashsize();
        bsz = 4 * PAGESIZE;
#if (MEMSTAT)
        g_memstat.nbhash += bsz;
#endif
        item = mapanon(0, bsz);
        first = (MEMPTR_T)item;
        if (item == MAP_FAILED) {

            crash(item != MAP_FAILED);
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
    MEMADR_T            adr = (MEMADR_T)ptr;
    MEMADR_T            page = adr >> PAGESIZELOG2;
    MEMUWORD_T          key = memhashptr(page) & (MEMHASHITEMS - 1);
    MEMADR_T            desc;
    MEMADR_T            bufval;
    MEMADR_T            upval;
    struct memhash     *blk;
    struct memhash     *prev;
    struct memhashitem *slot;
    struct memhashitem *src;
    MEMUWORD_T          lim;
    MEMUWORD_T          n;
    MEMUWORD_T          found;

//    fprintf(stderr, "locking hash chain %lx\n", key);
#if (!MEMLFHASH)
    memlkbit(&g_mem.hash[key].chain);
#endif
    upval = (MEMADR_T)g_mem.hash[key].chain;
#if (!MEMLFHASH)
    upval &= ~MEMLKBIT;
#endif
    found = 0;
    blk = (struct memhash *)upval;
    while ((blk) && !found) {
        lim = blk->ntab;
        src = blk->tab;
        prev = NULL;
        do {
            n = min(lim, 8);
            switch (n) {
                case 8:
                    slot = &src[7];
                    if (slot->page == page) {
                        found++;
                        
                        break;
                    }
                case 7:
                    slot = &src[6];
                    if (slot->page == page) {
                        found++;
                        
                        break;
                    }
                case 6:
                    slot = &src[5];
                    if (slot->page == page) {
                        found++;
                        
                        break;
                    }
                case 5:
                    slot = &src[4];
                    if (slot->page == page) {
                        found++;
                        
                        break;
                    }
                case 4:
                    slot = &src[3];
                    if (slot->page == page) {
                        found++;
                        
                        break;
                    }
                case 3:
                    slot = &src[2];
                    if (slot->page == page) {
                        found++;
                        
                        break;
                    }
                case 2:
                    slot = &src[1];
                    if (slot->page == page) {
                        found++;
                        
                        break;
                    }
                case 1:
                    slot = &src[0];
                    if (slot->page == page) {
                        found++;
                        
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
            desc |= id;
            if (blk) {
                prev = NULL;
                do {
                    n = blk->ntab;
                    if (n < MEMHASHITEMS) {
                        slot = &blk->tab[n];
                        found++;
                        n++;
#if defined(MEMHASHNREF) && (MEMHASHNREF)
                        slot->nref = 1;
#endif
#if defined(MEMHASHNACT) && (MEMHASHNACT)
                        slot->nact = 1;
#endif
                        slot->page = page;
                        blk->ntab = n;
                        slot->val = desc;
#if (MEMDEBUG)
                        crash(slot != NULL);
#endif
                        slot->val = desc;
                        if (prev) {
                            prev->chain = blk->chain;
                            blk->chain = (struct memhash *)upval;
#if (MEMHASHLOCK)
                            g_mem.hash[key].chain = blk;
#elif (MEMNEWHASH)
#if (!MEMLFHASH)
                            memrelbit(&g_mem.hash[key].chain);
#endif
#else
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
                    if (!found) {
                        prev = blk;
                        blk = blk->chain;
                    }
                } while (!found && (blk));
            }
            if (!found) {
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
                blk->list = &g_mem.hash[key];
                slot->page = page;
                slot->val = desc;
#if (MEMHASHLOCK)
                g_mem.hash[key].chain = blk;
                memrellk(&g_mem.hash[key].lk);
#else
                m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                            (m_atomic_t)blk);
#endif
            } else {
#if (!MEMLFHASH)
                memrelbit(&g_mem.hash[key].chain);
#endif
            }
#if (MEMDEBUG)
            crash(desc != 0);
#endif
            
            return desc;
        }
    }
    desc = slot->val;
//    upval &= ~MEMLKBIT;
    bufval = desc;
#if defined(MEMHASHNACT) && (MEMHASHNACT)
    slot->nact++;
#endif
//    desc &= ~MEMPAGEINFOMASK;
    if (op == MEMHASHDEL) {
        bufval &= ~MEMPAGEINFOMASK;
#if defined(MEMHASHNREF) && (MEMHASHNREF)
        slot->nref--;
#endif
        id = desc & MEMPAGEINFOMASK;
        n = blk->ntab;
        memrelblk(ptr, (struct membuf *)bufval, id);
#if (MEMHASHNREF)
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
                src = &blk->tab[n];
                slot->nref = src->nref;
#if defined(MEMHASHNACT) && (MEMHASHNACT)
                slot->nact = src->nact;
#endif
                n--;
//                upval &= ~MEMLKBIT;
                slot->page = src->page;
                slot->val = src->val;
                blk->ntab = n;
            }
        }
#endif
#if (MEMDEBUG)
        crash(desc != 0);
#endif
#if (!MEMLFHASH)
        memrelbit(&g_mem.hash[key].chain);
#endif
        
        return bufval;
    } else if (op == MEMHASHADD) {
#if 0
        if (op == MEMHASHCHK) {
            desc = MEMHASHFOUND;
        }
#endif
#if defined(MEMHASHNREF) && (MEMHASHNREF)
        slot->nref += op;       // zero for MEMHASHCHK
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

#endif

static MEMADR_T
memopenbuf(volatile struct membkt *bkt)
{
    MEMADR_T upval;

#if (MEMDEBUGDEADLOCK)
    memlkbitln(bkt);
#else
    memlkbit(&bkt->list);
#endif
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    if (!upval) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(bkt);
#else
        memrelbit(&bkt->list);
#endif
    }

    return upval;
}

MEMPTR_T
memtryblk(MEMWORD_T slot, MEMWORD_T type,
          MEMUWORD_T size, MEMUWORD_T align,
          volatile struct membkt *tbkt, volatile struct membkt *gbkt)
{
//    MEMADR_T                upval = (bkt1) ? bkt1->list : 0;
    MEMADR_T                upval = (tbkt) ? (MEMADR_T)tbkt->list : 0;
    struct membuf          *buf = (struct membuf *)upval;
    MEMPTR_T                ptr = NULL;
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    MEMUWORD_T              flg = 0;
#endif
    MEMWORD_T               nblk;
    volatile struct membkt *dest = (tbkt) ? tbkt : gbkt;

    if ((buf) && (tbkt)) {
        ptr = memgetblktls(buf, tbkt, size, align);
    } else {
        upval = memopenbuf(gbkt);
        buf = (struct membuf *)upval;
        nblk = memgetnbufblk(type, slot);
        if (buf) {
            if (tbkt) {
                ptr = memgetblkglob(buf, gbkt, size, align, tbkt);
            } else {
                ptr = memgetblkglob(buf, gbkt, size, align, NULL);
            }
        } else {
            if (type == MEMSMALLBUF) {
                buf = memallocsmallbuf(slot, nblk);
                if (buf) {
                    ptr = meminitsmallbuf(buf, size, align, nblk);
                }
            } else if (type == MEMPAGEBUF) {
                buf = memallocpagebuf(slot, nblk);
                if (buf) {
                    ptr = meminitpagebuf(buf, size, align, nblk);
                }
            } else {
                buf = memallocbigbuf(slot, nblk);
                if (buf) {
                    ptr = meminitbigbuf(buf, size, align, nblk);
                }
            }
            if (ptr) {
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
                if (type == MEMSMALLBUF) {
                    flg = buf->flg;
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
#endif
                if (nblk > 1) {
                    if (dest == tbkt) {
                        upval = (MEMADR_T)dest->list;
                    } else {
#if (MEMDEBUGDEADLOCK)
                        memlkbitln(dest);
#else
                        memlkbit(&dest->list);
#endif
                        upval = (MEMADR_T)dest->list;
                        upval &= ~MEMLKBIT;
                    }
                    buf->prev = NULL;
                    buf->bkt = dest;
                    if (upval) {
                        ((struct membuf *)upval)->prev = buf;
                    }
                    buf->next = (struct membuf *)upval;
#if (MEMDEBUGDEADLOCK)
                    dest->line = __LINE__;
#endif
                    dest->nbuf++;
                    if (type != MEMBIGBUF) {
                        dest->list = buf;
                    } else {
                        m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)buf);
                    }
                }
            }
        }
    }
    
    return ptr;
}

MEMPTR_T
memgetblk(MEMWORD_T slot, MEMWORD_T type, MEMUWORD_T size, MEMUWORD_T align)
{
    MEMPTR_T       ptr = NULL;
    
    if (type == MEMSMALLBUF) {
        ptr = memtryblk(slot, MEMSMALLBUF,
                        size, align,
                        &g_memtls->smallbin[slot], &g_mem.smallbin[slot]);
#if (MEMDEBUG)
        crash(ptr != NULL);
#endif
    } else if (type == MEMPAGEBUF) {
        ptr = memtryblk(slot, MEMPAGEBUF,
                        size, align,
                        &g_memtls->pagebin[slot], &g_mem.pagebin[slot]);
#if (MEMDEBUG)
        crash(ptr != NULL);
#endif
    } else {
        ptr = memtryblk(slot, MEMBIGBUF,
                        size, align,
                        NULL, &g_mem.bigbin[slot]);
#if (MEMDEBUG)
        crash(ptr != NULL);
#endif
    }
#if (MEMDEBUG)
    crash(ptr != NULL);
#endif

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
    buf->bkt = NULL;
    src->nbuf--;
    m_syncwrite((m_atomic_t)&src->list, (m_atomic_t)head);
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
    buf->bkt = NULL;
    src->nbuf--;
#if (MEMDEBUGDEADLOCK)
    src->line = __LINE__;
#endif
    m_syncwrite((m_atomic_t)&src->list, (m_atomic_t)head);

    return;
}

void
memrelblk(void *ptr, struct membuf *buf, MEMWORD_T id)
{
    volatile struct membkt *gbkt = NULL;
    volatile struct membkt *bkt = buf->bkt;
    MEMUWORD_T              type;
    MEMUWORD_T              slot;
    MEMADR_T                upval;
    MEMWORD_T               nblk;
    MEMWORD_T               nfree;
    MEMWORD_T               nbuf;
    MEMWORD_T               glob;
    MEMWORD_T               recl = 0;

#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif
    glob = (bkt >= &g_mem.bigbin[0] && bkt <= &g_mem.smallbin[MEMMAXSMALLSLOT]);
    if (bkt) {
        if (glob) {
#if (MEMDEBUGDEADLOCK)
            memlkbitln(bkt);
#else
            memlkbit(&bkt->list);
#endif
        }
        if (!m_cmpswapptr((m_atomic_t *)&buf->bkt, (m_atomic_t *)bkt, NULL)) {
            if (glob) {
#if (MEMDEBUGDEADLOCK)
                memrelbitln(bkt);
#else
                memrelbit(&bkt->list);
#endif
            }
            
            return;
        }
    }
    type = memgetbuftype(buf);
    slot = memgetbufslot(buf);
    nfree = memgetbufnfree(buf);
    nblk = memgetbufnblk(buf);
    if (glob) {
        gbkt = bkt;
    } else if (type == MEMSMALLBUF) {
        gbkt = &g_mem.smallbin[slot];
    } else if (type == MEMPAGEBUF) {
        gbkt = &g_mem.pagebin[slot];
    } else {
        gbkt = &g_mem.bigbin[slot];
    }
#if (MEMDEBUG)
    crash(nfree < nblk);
#endif
    nfree++;
    if (type != MEMPAGEBUF) {
        id = membufblkid(buf, ptr);
        membufsetadr(buf, ptr, NULL);
    } else {
        membufsetpageadr(buf, id, NULL);
    }
    setbit(buf->freemap, id);
    memsetbufnfree(buf, nfree);
    VALGRINDPOOLFREE(buf->base, ptr);
    if (nfree == 1) {
        /* acquire lock for the global list */
#if (MEMDEBUGDEADLOCK)
        memlkbitln(gbkt);
#else
        memlkbit(&gbkt->list);
#endif
        /* add buffer in front of the global list */
        upval = (MEMADR_T)gbkt->list;
        buf->prev = NULL;
        upval &= ~MEMLKBIT;
        buf->bkt = gbkt;
        if (upval) {
            ((struct membuf *)upval)->prev = buf;
        }
        buf->next = (struct membuf *)upval;
        gbkt->nbuf++;
#if (MEMDEBUGDEADLOCK)
        gbkt->line = __LINE__;
#endif
        /* this will unlock the list (set the low-bit to zero) */
        m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
    } else if (nfree == nblk) {
        /* queue or reclaim a free buffer */
        nbuf = memgetnbufglob(type, slot);
        if (!glob && bkt->nbuf > nbuf) {
            memdequeuebuftls(buf, bkt);
            if (type == MEMSMALLBUF || gbkt->nbuf < nbuf) {
                /* acquire lock for the global list */
#if (MEMDEBUGDEADLOCK)
                memlkbitln(gbkt);
#else
                memlkbit(&gbkt->list);
#endif
                /* add buffer in front of the global list */
                upval = (MEMADR_T)gbkt->list;
                buf->prev = NULL;
                upval &= ~MEMLKBIT;
                buf->bkt = gbkt;
                if (upval) {
                    ((struct membuf *)upval)->prev = buf;
                }
                buf->next = (struct membuf *)upval;
                gbkt->nbuf++;
#if (MEMDEBUGDEADLOCK)
                gbkt->line = __LINE__;
#endif
                /* this will unlock the list (set the low-bit to zero) */
                m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
            } else if (gbkt->nbuf >= nbuf) {
                recl = 1;
            }
        } else if ((glob) && type != MEMSMALLBUF && gbkt->nbuf > nbuf) {
            memdequeuebufglob(buf, bkt);
            recl = 1;
        }
        if (recl) {
            /* unmap the buffer */
            VALGRINDRMPOOL(buf->base);
            unmapanon(buf, buf->size);
#if (MEMSTAT)
            g_memstat.nbunmap += buf->size;
#endif
        }
    } else if (bkt == gbkt) {
        /* no need to reclaim or requeue, just unlock if on global list */
#if (MEMDEBUGDEADLOCK)
        memrelbitln(bkt);
#else
        memrelbit(&bkt->list);
#endif
    }
        
    return;
}

