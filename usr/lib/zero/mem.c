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
//    MEMWORD_T               bufsz;
    MEMWORD_T               nbuf;
    MEMWORD_T               nb;
    struct membuf          *buf;
    struct membuf          *head;
    MEMADR_T                upval;

    if (g_memtls) {
#if 0
        bufsz = g_memtls->nbytetab[MEMSMALLBUF];
#endif
        for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
            src = &g_memtls->smallbin[slot];
            dest = &g_mem.deadsmall[slot];
//            dest = &g_mem.smallbin[slot];
#if 0
            dest = &g_mem.smallbin[slot];
#endif
            head = src->list;
            nbuf = 0;
            if (head) {
                buf = head;
                nbuf++;
#if (MEMBUFRELMAP)
//                membuffreerel(buf);
#endif
                buf->tls = NULL;
                while (buf->next) {
                    buf = buf->next;
                    nbuf++;
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
            dest = &g_mem.deadpage[slot];
//            dest = &g_mem.pagebin[slot];
#if 0
            dest = &g_mem.pagebin[slot];
#endif
            head = src->list;
            nbuf = 0;
            if (head) {
                buf = head;
                nbuf++;
#if (MEMBUFRELMAP)
//                membuffreerel(buf);
#endif
                buf->tls = NULL;
                while (buf->next) {
                    buf = buf->next;
                    nbuf++;
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
    spinunlk(&g_mem.initlk);

    return g_memtls;
}

static void
memprefork(void)
{
    MEMWORD_T slot;

    spinlk(&g_mem.initlk);
    memgetlk(&g_mem.heaplk);
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
            memlkbit(&g_mem.hash[slot].chain);
        }
    }
    for (slot = 0 ; slot < MEMSMALLSLOTS ; slot++) {
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.smallbin[slot]);
#else
        memlkbit(&g_mem.smallbin[slot].list);
#endif
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.deadsmall[slot]);
#else
        memlkbit(&g_mem.deadsmall[slot].list);
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
#if (MEMDEBUGDEADLOCK)
        memlkbitln(&g_mem.deadpage[slot]);
#else
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
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.deadpage[slot]);
#else
        memrelbit(&g_mem.deadpage[slot].list);
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
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.deadsmall[slot]);
#else
        memrelbit(&g_mem.deadsmall[slot].list);
#endif
#if (MEMDEBUGDEADLOCK)
        memrelbitln(&g_mem.smallbin[slot]);
#else
        memrelbit(&g_mem.smallbin[slot].list);
#endif
    }
    if (g_mem.hash) {
        for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
            memrelbit(&g_mem.hash[slot].chain);
        }
    }
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

//    fprintf(stderr, "MEMHASHARRAYITEMS == %d\n", MEMHASHARRAYITEMS);
//    spinlk(&g_mem.initlk);
#if 0
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

        crash(ptr != MAP_FAILED);
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
    memsetbuf(ptr, buf, 0);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif

    return ptr;
}

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

    return;
}
#if (MEMHASHSUBTABS)
static void
meminithashsubtab(MEMPTR_T data)
{
    struct memhashsubitem *item = (struct memhash *)data;

    item->adr = 0;
    item->val = 0;

    return;
}
#endif
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
    volatile struct memtls *tls;
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

    fprintf(stderr, "locking hash chain %lx\n", key);
    memlkbit(&g_mem.hash[key].chain);
    upval = (MEMADR_T)g_mem.hash[key].chain;
    upval &= ~MEMLKBIT;
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
        do {
            n = min(lim, 16);
            switch (n) {
                /*
                 * if found, the mask will be -1 (all 1-bits), and val will be
                 * the item address
                 * if not found, the mask will be 0 and so will val/slot
                 */
                case 16:
                    item = &src[15];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 15:
                    item = &src[14];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 13:
                    item = &src[12];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                    if (slot) {

                        break;
                    }
                case 12:
                    item = &src[11];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 10:
                    item = &src[9];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                case 9:
                    item = &src[8];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                    if (slot) {

                        break;
                    }
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
                case 6:
                    item = &src[5];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
                    if (slot) {

                        break;
                    }
                case 5:
                    item = &src[4];
                    mask = -((MEMADRDIFF_T)(item->adr == page));
                    val = (MEMADR_T)((MEMADR_T)mask & (MEMADR_T)item);
                    slot = (struct memhashitem *)val;
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
            if (!slot && (n)) {
                prev = blk;
                blk = blk->chain;
            } else if (slot) {
                found++;
                desc = slot->val;
                
                break;
            } else {
                
                break;
            }
        } while (1);
    }
//    upval = (MEMADR_T)g_mem.hash[key].chain;
    if (!found) {
        if (op == MEMHASHDEL || op == MEMHASHCHK) {
            memrelbit(&g_mem.hash[key].chain);

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
                            fprintf(stderr, "releasing hash chain %lx\n", key);
#if (MEMHASHLOCK)
                            g_mem.hash[key].chain = blk;
#elif (MEMNEWHASH)
                            m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                        (m_atomic_t)blk);
#endif
                        } else {
                            fprintf(stderr, "releasing hash chain %lx\n", key);
                            memrelbit(&g_mem.hash[key].chain);
                        }

                        return desc;
                    }
                    prev = blk;
                    blk = blk->chain;
                } while (blk);
            } else {
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
                fprintf(stderr, "releasing hash chain %lx\n", key);
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
            }

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
                    if (prev) {
                        prev->chain = blk->chain;
                        fprintf(stderr, "releasing hash chain %lx\n", key);
                        memrelbit(&g_mem.hash[key].chain);
                    } else {
                        fprintf(stderr, "releasing hash chain %lx\n", key);
                        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                    (m_atomic_t)blk->chain);
                    }
                    membufhashitem(blk);

                    return desc;
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
            fprintf(stderr, "releasing hash chain %lx\n", key);
            memrelbit(&g_mem.hash[key].chain);

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
            fprintf(stderr, "releasing hash chain %lx\n", key);
            m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                        (m_atomic_t)blk);
        } else {
            fprintf(stderr, "releasing hash chain %lx\n", key);
            memrelbit(&g_mem.hash[key].chain);
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
    buf = bkt->list;
    if (!buf) {

        return NULL;
    }
    id = membufgetfree(buf);
    nfree = memgetbufnfree(buf);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
    crash(nfree > 0 && nfree <= nblk);
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
        m_syncwrite((m_atomic_t *)&bkt->list, buf->next);
        buf->tls = NULL;
        buf->prev = NULL;
        buf->next = NULL;
    }
    
    return ptr;
}

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
#if (MEMBUFRELMAP)
    membuffreerel(buf);
#endif
    id = membufgetfree(buf);
    nfree = memgetbufnfree(buf);
#if (MEMDEBUG)
    nblk = memgetbufnblk(buf);
    crash(nfree > 0 && nfree <= nblk);
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
        memsetbuf(ptr, buf, -1);
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
        m_syncwrite((m_atomic_t *)&bkt->list, buf);
    } else {
        buf->tls = NULL;
        buf->prev = NULL;
        buf->next = NULL;
    }        

    return ptr;
}

MEMPTR_T
memgetblkglob(MEMWORD_T type, MEMWORD_T slot, MEMWORD_T size, MEMWORD_T align)
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
    crash(nfree > 0 && nfree <= nblk);
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
#if (MEMBUFSTACK) && 0
    memprintbufstk(buf, "MEMGETBLKGLOB\n");
#endif
    if (!nfree) {
        /* buf shall be disconnected from all lists */
        if (buf->next) {
            buf->next->prev = NULL;
        }
#if (MEMDEBUGDEADLOCK)
        bkt->line = __LINE__;
#endif
        bkt->nbuf--;
        m_syncwrite((m_atomic_t *)&bkt->list, (m_atomic_t)buf->next);
        m_clrbit((m_atomic_t *)&buf->info, MEMBUFGLOBBITID);
        buf->prev = NULL;
        buf->next = NULL;
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
                ptr = meminitsmallbuf(buf, slot, size, align, nblk);
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
            dest->nbuf++;
            if (type != MEMBIGBUF) {
                buf->tls = tls;
            } else {
                m_setbit(&buf->info, MEMBUFGLOBBITID);
            }
            m_syncwrite((m_atomic_t *)&dest->list,
                        (m_atomic_t)buf);
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
    if (buf->info & MEMBUFGLOBBIT) {
        isglob = 1;
    }
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
        if (isglob) {
#if (MEMDEBUGDEADLOCK)
            memrelbitln(gbkt);
#else
            memrelbit(&gbkt->list);
#endif
        }

        return;
    }
#if (MEMBUFSTACK) && 0
    memprintbufstk(head, "MEMRELBLK\n");
#endif
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
        gbkt->nbuf++;
        m_setbit(&buf->info, MEMBUFGLOBBITID);
            /* this will unlock the list (set the low-bit to zero) */
        m_syncwrite((m_atomic_t *)&gbkt->list, (m_atomic_t *)buf);
        
        return;
    } else if (nfree == nblk) {
        /* queue or reclaim a free buffer */
        if (!isglob) {
            if (!tls || tls != g_memtls) {

                return;
            }
            if (tbkt) {
                if (type == MEMSMALLBUF) {
                    if (tbkt->nbuf > 2) {
                        memdequeuebuftls(buf, tbkt);
                    } else {
                        
                        return;
                    }
                } else if (type == MEMPAGEBUF) {
                    if (tbkt->nbuf > 2) {
                        memdequeuebuftls(buf, tbkt);
                    } else {
                        
                        return;
                    }
                }
            }
#if (MEMDEBUGDEADLOCK)
            memlkbitln(gbkt);
#else
            memlkbit(&gbkt->list);
#endif
        }
        if (gbkt->nbuf > 4) {
            if (isglob) {
                memdequeuebufglob(buf, gbkt);
#if (MEMDEBUGDEADLOCK)
                memrelbitln(gbkt);
#else
                memrelbit(&gbkt->list);
#endif
            }
#if (MEMSTAT)
            g_memstat.nbunmap += buf->size;
#endif
            VALGRINDRMPOOL(buf->base);
            unmapanon(buf, buf->size);

            return;
        }  else if (!isglob) {
            /* add buffer in front of global list */
            upval = (MEMADR_T)gbkt->list;
            buf->prev = NULL;
            upval &= ~MEMLKBIT;
            head = (struct membuf *)upval;
            if (upval) {
                head->prev = buf;
            }
            buf->next = head;
#if (MEMDEBUGDEADLOCK)
            gbkt->line = __LINE__;
#endif
            gbkt->nbuf++;
            m_setbit(&buf->info, MEMBUFGLOBBITID);
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
    } else if (isglob) {
#if (MEMDEBUGDEADLOCK)
        memrelbitln(gbkt);
#else
        memrelbit(&gbkt->list);
#endif
    }

    return;
}

