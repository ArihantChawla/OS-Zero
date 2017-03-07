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
                membuffreerel(head);
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
                    membuffreerel(buf);
#endif
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
                membuffreerel(head);
#endif
                buf = head;
                nbuf++;
                buf->tls = NULL;
                while (buf->next) {
                    nbuf++;
                    buf = buf->next;
#if (MEMBUFRELMAP)
                    membuffreerel(buf);
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
                dest->nbuf += nbuf;
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
            }
        }
#if 0
        nb = g_mem.nbytetab[MEMSMALLBUF];
        nb += bufsz;
        g_mem.nbytetab[MEMSMALLBUF] = bufsz;
#endif
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

    spinlk(&g_mem.initlk);
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    memgetlk(&g_mem.heaplk);
#endif
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
//            memlkbit(&g_mem.hash[slot].chain);
            fmtxlk(&g_mem.hash[slot].lk);
        }
    }
    for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.smallbin[slot]);
#else
        memlkbit(&g_mem.smallbin[slot].list);
#endif
#if (MEMDEADBINS)
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.deadsmall[slot]);
#else
        memlkbit(&g_mem.deadsmall[slot].list);
#endif
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
#if (MEMDEADBINS)
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.deadpage[slot]);
#else
        memlkbit(&g_mem.deadpage[slot].list);
#endif
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
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.deadpage[slot]);
#else
        memrelbit(&g_mem.deadpage[slot].list);
#endif
#endif
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
#if (MEMDEADBINS)
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.deadsmall[slot]);
#else
        memrelbit(&g_mem.deadsmall[slot].list);
#endif
#endif
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.smallbin[slot]);
#else
        memrelbit(&g_mem.smallbin[slot].list);
#endif
    }
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
//            memrelbit(&g_mem.hash[slot].chain);
            fmtxunlk(&g_mem.hash[slot].lk);
        }
    }
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
#if !defined(MEMNOSBRK) || !(MEMNOSBRK)
    void       *heap;
    intptr_t    ofs;
#endif
    void       *ptr;

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
    ptr = mapanon(0, MEMHASHITEMS * sizeof(struct memhashlist));
#if (MEMSTAT)
    g_memstat.nbhashtab = MEMHASHITEMS * sizeof(struct memhashlist);
#endif
    if (ptr == MAP_FAILED) {
        crash(ptr == MAP_FAILED);
    }
    g_mem.hash = ptr;
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
    buf->info = flg;    // possible MEMHEAPBIT
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
    MEMPTR_T  adr = (MEMPTR_T)buf;
    MEMPTR_T  ptr = adr + membufblkofs(nblk);
    MEMWORD_T bsz = MEMWORD(1) << slot;

    /* initialise freemap */
    buf->base = ptr;
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    membufinitfree(buf);
#else
    membufinitfree(buf, MEMSMALLBUF, slot, nblk);
#endif
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    nblk--;
    memsetbufnfree(buf, nblk);
#else
    membufgetfree(buf);
#endif
    VALGRINDMKPOOL(ptr, 0, 0);
//    ptr = memcalcadr(ptr, size, align, 0);
    ptr = memcalcadr(ptr, size, bsz, align);
#if 0
    ptr = memalignptr(ptr, align);
#endif
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
    MEMPTR_T adr = (MEMPTR_T)buf;
    MEMPTR_T ptr = adr + membufblkofs(nblk);
    MEMWORD_T bsz = PAGESIZE + slot * PAGESIZE;

    /* initialise freemap */
    buf->base = ptr;
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    membufinitfree(buf);
#else
    membufinitfree(buf, MEMPAGEBUF, slot, nblk);
#endif
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    nblk--;
    memsetbufnfree(buf, nblk);
#else
    membufgetfree(buf);
#endif
    VALGRINDMKPOOL(ptr, 0, 0);
//    ptr = memcalcadr(ptr, size, align, 0);
    ptr = memcalcadr(ptr, size, bsz, align);
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
    MEMPTR_T  adr = (MEMPTR_T)buf;
    MEMPTR_T  ptr = adr + membufblkofs(nblk);
    MEMWORD_T bsz = MEMWORD(1) << slot;

    buf->base = ptr;
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    membufinitfree(buf);
#else
    membufinitfree(buf, MEMBIGBUF, slot, nblk);
#endif
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    nblk--;
    memsetbufnfree(buf, nblk);
#else
    membufgetfree(buf);
#endif
    VALGRINDMKPOOL(ptr, 0, 0);
//    ptr = memcalcadr(ptr, size, align, 0);
    ptr = memcalcadr(ptr, size, bsz, align);
    memsetbuf(ptr, buf, 0);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

#if (MEMHASHSUBTABS)

static struct memhashitem *
memgethashsubtab(struct memhashslot *slot)
{
    struct memhash *item;
    MEMPTR_T        first;
    MEMWORD_T       bsz;
    MEMADR_T        upval;

    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    if (upval) {
        item = (struct memnhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item->chain);
    } else {
        bsz = rounduppow2(memhashtabsize(), PAGESIZE);
        item = mapanon(0, bsz);
        if (item == MAP_FAILED) {

            abort();
        }
        first = (MEMPTR_T)item;
        if (item == MAP_FAILED) {

            abort();
        }
        slot->itab = item;
    }

    return item;
}

#if 0
static void
memrelhashsubtab(struct memhashslot *slot)
{
    MEMWORD_T bsz = rounduppow2(memhashtabsize(), PAGESIZE);

    unmapanon(0, bsz);
    slot->itab = NULL;

    return;
}
#endif

#endif

#if (MEMNEWHASHTAB) && (MEMHASHSUBTABS)
static void
meminithashitem(MEMPTR_T data)
{
    struct memhash *item = (struct memhash *)data;
    MEMUWORD_T     *uptr;

    if (!memgethashsubtab(item)) {

        abort();
    }
#if 0
    uptr = (MEMUWORD_T *)data;
    data += offsetof(struct memhash, data);
    item->chain = NULL;
    item->ntab = 0;
    item->itab = (struct memhashitem *)uptr;
#endif

    return;
}
#else
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

    memlkbit(&g_mem.hashbuf);
    upval = (MEMADR_T)g_mem.hashbuf;
    upval &= ~MEMLKBIT;
    if (upval) {
        item = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)item->chain);
//        meminithashitem(item);
    } else {
#if (MEMBIGHASHTAB)
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
membufop(MEMPTR_T ptr, MEMWORD_T op, struct membuf *buf, MEMWORD_T id)
{
    volatile struct memtls *tls;
    MEMPTR_T                adr = ptr;
    MEMADR_T                page = (MEMADR_T)adr >> PAGESIZELOG2;
#if (MEMHASHSUBTABS)
    MEMADR_T                ofs = ((MEMADR_T)adr
                                   >> (PAGESIZELOG2 + MEMALIGNSHIFT));
#endif
    MEMUWORD_T              key = memhashptr(page) & (MEMHASHITEMS - 1);
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
#if (MEMBFHASH)
    struct memhashitem     *item;
    MEMADRDIFF_T            mask;
#endif

//    fprintf(stderr, "LOCK: %lx\n", key);
    fmtxlk(&g_mem.hash[key].lk);
#if (!MEMHASHSUBTABS)
    upval = (MEMADR_T)g_mem.hash[key].chain;
#else
#if (MEMHASHSUBTABS)
    ofs &= (MEMHASHSUBTABITEMS - 1);
#endif
//    upval &= ~MEMLKBIT;
    if (!upval) {
        if (op != MEMHASHADD) {
            fmtxunlk(&g_mem.hash[key].lk);

            return MEMHASHNOTFOUND;
        } else {
            blk = memgethashitem();
            if (!blk) {

                return MEMHASHNOTFOUND;
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
#if (MEMDEBUG)
            crash(slot == NULL);
#endif
//                        fprintf(stderr, "REL: %lx\n", key);
            blk->chain = NULL;
            g_mem.hash[key].chain = blk;
            fmtxunlk(&g_mem.hash[key].lk);

            return desc;
    }
#endif
    dest = NULL;
    prev = NULL;
#if (!MEMHASHSUBTABS)
    upval &= ~MEMLKBIT;
#endif
    desc |= id;
    slot = NULL;
#if (MEMHASHSUBTABS)
    upval = (MEMADR_T)&blk->tab[ofs];
#endif
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
        if (op == MEMHASHDEL || op == MEMHASHCHK) {
//            fprintf(stderr, "REL: %lx\n", key);
            fmtxunlk(&g_mem.hash[key].lk);

            return MEMHASHNOTFOUND;
        } else if (dest) {
            n = *cnt;
            dest->adr = page;
            n++;
            dest->val = desc;
            *cnt = n;
            fmtxunlk(&g_mem.hash[key].lk);

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
                            g_mem.hash[key].chain = blk;
                        }
                        fmtxunlk(&g_mem.hash[key].lk);
                        
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
#if defined(MEMHASHSUBTABS) && (MEMHASHSUBTABS)
            if (!memgethashsubtab(blk)) {

                abort();
            }
#endif
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
            g_mem.hash[key].chain = blk;
            fmtxunlk(&g_mem.hash[key].lk);
#if (MEMDEBUG)
            crash(desc == 0);
#endif

            return desc;
        }
    } else {
#if defined(MEMHASHNACT) && (MEMHASHNACT)
        slot->nact++;
#endif
        if (op == MEMHASHDEL) {
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
                        g_mem.hash[key].chain = blk;
                    }
                    fmtxunlk(&g_mem.hash[key].lk);
                    membufhashitem(blk);

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
#if (MEMDEBUG)
            crash(desc == 0);
#endif
//            fprintf(stderr, "REL: %lx\n", key);
            fmtxunlk(&g_mem.hash[key].lk);

            return desc;
#if defined(MEMHASHNREF) && (MEMHASHNREF)
        } else if (op == MEMHASHADD) {
            slot->nref++;
#endif
        }
    }
//    fprintf(stderr, "REL: %lx\n", key);
    if (prev) {
        prev->chain = blk->chain;
        blk->chain = (struct memhash *)upval;
    }
    fmtxunlk(&g_mem.hash[key].lk);
#if (MEMDEBUG)
    crash(desc == 0);
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
    struct membuf          *buf;
    MEMPTR_T                ptr;
    MEMPTR_T                adr;
    MEMWORD_T               bsz;
    MEMWORD_T               nfree;
#if (MEMDEBUG)
    MEMWORD_T               nblk;
#endif
    MEMWORD_T               id;

    if (type == MEMSMALLBUF) {
        bsz = MEMWORD(1) << slot;
        bkt = &g_memtls->smallbin[slot];
    } else {
        bsz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_memtls->pagebin[slot];
    }
//    memprintbufstk(buf, "MEMGETBLKTLS\n");
    buf = bkt->list;
    if (!buf) {

        return NULL;
    }
    id = membufgetfree(buf);
    nfree = memgetbufnfree(buf);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
#if defined(MEMBUFSTACK) && (MEMBUFSTACK)
    crash(nfree <= 0 || nfree >= nblk);
#else
    crash(nfree <= 0 && nfree >= nblk);
#endif
#endif
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(buf, id);
    } else {
        adr = membufpageadr(buf, id);
    }
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    nfree--;
#endif
//            ptr = memcalcadr(buf, adr, size, align, id);
    ptr = memcalcadr(adr, size, bsz, align);
    if (type != MEMPAGEBUF) {
        memsetbuf(ptr, buf, 0);
    } else {
        memsetbuf(ptr, buf, id);
    }
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    memsetbufnfree(buf, nfree);
#endif
    VALGRINDPOOLALLOC(buf->base, ptr, size);
    if (!nfree) {
        /* buf shall be disconnected from all lists */
        if (buf->next) {
            buf->next->prev = NULL;
        }
#if (MEMDEBUGDEADLOCK)
        bkt->line = __LINE__;
#endif
        bkt->nbuf--;
        bkt->list = buf->next;
#if (!MEMEMPTYTLS)
        buf->tls = NULL;
#endif
        buf->prev = NULL;
        buf->next = NULL;
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
    MEMWORD_T               nfree;
#if (MEMDEBUG)
    MEMWORD_T               nblk;
#endif
    MEMWORD_T               id;
    MEMADR_T                upval;

    if (type == MEMSMALLBUF) {
        bsz = MEMWORD(1) << slot;
        bkt = &g_mem.deadsmall[slot];
    } else {
        bsz = PAGESIZE + PAGESIZE * slot;
        bkt = &g_mem.deadpage[slot];
    }
#if (MEMDEBUGDEADLOCK)
    memlkbitln(bkt);
#else
    memlkbit(&bkt->list);
#endif
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    buf = (struct membuf *)upval;
    if (!buf) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(bkt);
#else
        memrelbit(&bkt->list);
#endif
        
        return NULL;
    }
#if (MEMBUFRELMAP) && 0
    membuffreerel(buf);
#endif
    id = membufgetfree(buf);
    nfree = memgetbufnfree(buf);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
    crash(nfree <= 0 && nfree >= nblk);
#endif
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(buf, id);
    } else {
        adr = membufpageadr(buf, id);
    }
#if !defined(MEMBUFSTACK) || (MEMBUFSTACK)
    nfree--;
#endif
//            ptr = memcalcadr(buf, adr, size, align, id);
    ptr = memcalcadr(adr, size, bsz, align);
    if (type != MEMPAGEBUF) {
        memsetbuf(ptr, buf, 0);
    } else {
        memsetbuf(ptr, buf, id);
    }
#if !defined(MEMBUFSTACK) || (MEMBUFSTACK)
    memsetbufnfree(buf, nfree);
#endif
    VALGRINDPOOLALLOC(buf->base, ptr, size);
    /* buf shall be disconnected from all lists */
    if (buf->next) {
        buf->next->prev = NULL;
    }
#if (MEMDEBUGDEADLOCK)
    bkt->line = __LINE__;
#endif
    bkt->nbuf--;
    m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf->next);
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
        buf->prev = NULL;
        buf->next = NULL;
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
#if (MEMDEBUGDEADLOCK)
    memlkbitln(bkt);
#else
    memlkbit(&bkt->list);
#endif
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    buf = (struct membuf *)upval;
    if (!buf) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(bkt);
#else
        memrelbit(&bkt->list);
#endif

        return NULL;
    }
    id = membufgetfree(buf);
    nfree = memgetbufnfree(buf);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
#if defined(MEMBUFSTACK) && (MEMBUFSTACK)
    crash(nfree <= 0 && nfree >= nblk);
#else
    crash(nfree <= 0 && nfree >= nblk);
#endif
#endif
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    nfree--;
#endif
    if (type != MEMPAGEBUF) {
        adr = membufblkadr(buf, id);
    } else {
        adr = membufpageadr(buf, id);
    }
#if !defined(MEMBUFSTACK) || (!MEMBUFSTACK)
    memsetbufnfree(buf, nfree);
#endif
//    ptr = memcalcadr(buf, adr, size, align, id);
    ptr = memcalcadr(adr, size, bsz, align);
    memsetbuf(ptr, buf, id);
#if (MEMTEST) && 0
    _memchkptr(buf, ptr);
#endif
    VALGRINDPOOLALLOC(buf->base, ptr, size);
    if (!nfree || type != MEMBIGBUF) {
        /* buf shall be disconnected from all lists */
        if (buf->next) {
            buf->next->prev = NULL;
        }
#if (MEMDEBUGDEADLOCK)
        bkt->line = __LINE__;
#endif
        bkt->nbuf--;
        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf->next);
//        m_clrbit((m_atomic_t *)&buf->info, MEMBUFGLOBBITID);
        buf->prev = NULL;
        buf->next = NULL;
    }
    if ((nfree) && type != MEMBIGBUF) {
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
#if (MEMDEADBINS)
        if (!ptr) {
            ptr = memgetblkdead(type, slot, size, align);
        }
#endif
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
            bktsz = PAGESIZE + (PAGESIZE << slot);
            buf = memallocpagebuf(slot, nblk);
            if (buf) {
                ptr = meminitpagebuf(buf, slot, size, align, nblk);
            }
        } else {
            bktsz = nblk << slot;
            buf = memallocbigbuf(slot, nblk);
            if (buf) {
                ptr = meminitbigbuf(buf, slot, size, align, nblk);
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
                buf->prev = NULL;
                if (head) {
                    head->prev = buf;
                }
                buf->next = head;
                dest->nbuf++;
                buf->tls = tls;
                dest->list = buf;
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
                buf->prev = NULL;
                if (head) {
                    head->prev = buf;
                }
                buf->next = head;
                dest->nbuf++;
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
    src->nbuf--;
    m_syncwrite((m_atomic_t *)&src->list, head);

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
    src->nbuf--;
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
    MEMWORD_T               lim;
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
#if (MEMDEBUGDEADLOCK)
        memlkbitln(gbkt);
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
    nfree = membufputfree(buf, id);
#endif
    if (nfree != 1 && nfree != nblk) {
        /* buffer not totally free or allocated */
        if (!tls) {
#if (MEMDEBUGDEADLOCK)
            memrelbitln(gbkt);
#else
            memrelbit(&gbkt->list);
#endif
        }

        return;
    } else if (nfree == 1) {
#if (MEMEMPTYTLS)
        if (type != MEMBIGBUF) {
            head = tbkt->list;
            if (head) {
                head->prev = buf;
            }
            buf->next = head;
            tbkt->nbuf++;
            tbkt->list = buf;
#if (!MEMDEADBINS)
            membuffreerel(buf);
#endif
        } else {
#if 0
            if (tls) {
#if (MEMDEBUGDEADLOCK)
                memlkbitln(gbkt);
#else
                memlkbit(&gbkt->list);
#endif
            }
#endif
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
            gbkt->nbuf++;
//        m_setbit(&buf->info, MEMBUFGLOBBITID);
            /* this will unlock the list (set the low-bit to zero) */
            m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
        }
#else /* !MEMEMPTYTLS */
        if (type != MEMBIGBUF) {
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
        gbkt->nbuf++;
//        m_setbit(&buf->info, MEMBUFGLOBBITID);
            /* this will unlock the list (set the low-bit to zero) */
        m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
#endif /* MEMEMPTYTLS */
        
        return;
    } else if (nfree != nblk && !tls) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(gbkt);
#else
        memrelbit(&gbkt->list);
#endif
    } else {
        /* nfree == nblk */
        /* queue or reclaim a free buffer */
        if (tls) {
            if (type != MEMBIGBUF) {
                if (tbkt->nbuf >= membktnbuftls(type, slot)) {
                    memdequeuebuftls(buf, tbkt);
                } else {
                    
                    return;
                }
            }
#if (MEMDEBUGDEADLOCK)
            memlkbitln(gbkt);
#else
            memlkbit(&gbkt->list);
#endif
        }
        if ((MEMUNMAP) && gbkt->nbuf >= membktnbufglob(type, slot)) {
            if (!tls) {
                memdequeuebufglob(buf, gbkt);
            }
#if (MEMDEBUGDEADLOCK)
            memrelbitln(gbkt);
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
            upval = (MEMADR_T)gbkt->list;
            buf->prev = NULL;
            upval &= ~MEMLKBIT;
            head = (struct membuf *)upval;
            if (upval) {
                head->prev = buf;
            }
            buf->next = head;
            buf->tls = NULL;
#if (MEMDEBUGDEADLOCK)
            gbkt->line = __LINE__;
#endif
            gbkt->nbuf++;
//            m_setbit(&buf->info, MEMBUFGLOBBITID);
            /* this will unlock the list (set the low-bit to zero) */
            m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);

            return;
        } else {
#if (MEMDEBUGDEADLOCK)
            memrelbitln(gbkt);
#else
            memrelbit(&gbkt->list);
#endif

            return;
        }
    }

    return;
}

