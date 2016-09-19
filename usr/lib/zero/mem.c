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
#include <zero/mem.h>
#include <zero/hash.h>
#if (MEM_LK_TYPE & MEM_LK_PRIO)
#include <zero/priolk.h>
#endif

static pthread_once_t               g_thronce = PTHREAD_ONCE_INIT;
static pthread_key_t                g_thrkey;
THREADLOCAL volatile struct memtls *g_memtls;
struct mem                          g_mem;
#if (MEMSTAT)
struct memstat                      g_memstat;
#endif

static void
memfreetls(void *arg)
{
    struct memtls *tls = g_memtls;
    void          *adr = arg;
    struct membkt *src;
    struct membkt *dest;
    MEMUWORD_T     slot;
    MEMUWORD_T     n;
    struct membuf *head;
    struct membuf *buf;
    MEMADR_T       upval;

    src = &tls->smallbin[0];
    dest = &g_mem.smallbin[0];
    for (slot = 0 ; slot < PTRBITS ; slot++) {
        memlkbit(&src->list);
        upval = (MEMADR_T)src->list;
        upval &= ~MEMLKBIT;
        head = (struct membuf *)upval;
        if (head) {
            buf = head;
            buf->bkt = dest;
            while (buf->next) {
                buf = buf->next;
                buf->bkt = dest;
            }
            n = src->nbuf;
            memlkbit(&dest->list);
            upval = (MEMADR_T)dest->list;
            upval &= ~MEMLKBIT;
            dest->nbuf += n;
            buf->prev = NULL;
            buf->next = (struct membuf *)upval;
            if (upval) {
                ((struct membuf *)upval)->prev = buf;
            }
            m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
        }
        memrelbit(&src->list);
        src++;
        dest++;
    }
    src = &tls->pagebin[0];
    dest = &g_mem.pagebin[0];
    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
        memlkbit(&src->list);
        upval = (MEMADR_T)src->list;
        upval &= ~MEMLKBIT;
        head = (struct membuf *)upval;
        if (head) {
            buf = head;
            buf->bkt = dest;
            while (buf->next) {
                buf = buf->next;
                buf->bkt = dest;
            }
            n = src->nbuf;
            memlkbit(&dest->list);
            upval = (MEMADR_T)dest->list;
            upval &= ~MEMLKBIT;
            dest->nbuf += n;
            buf->prev = NULL;
            buf->next = (struct membuf *)upval;
            if (upval) {
                ((struct membuf *)upval)->prev = buf;
            }
            m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)head);
        }
        memrelbit(&src->list);
        src++;
        dest++;
    }
    unmapanon(adr, memtlssize());

    return;
}

#if (MEM_LK_TYPE == MEM_LK_PRIO)
static unsigned long
memgetprioval(void)
{
    unsigned long val;

//    fmtxlk(&g_mem.priolk);
    val = g_mem.prioval;
    val++;
    val &= sizeof(long) * CHAR_BIT - 1;
    g_mem.prioval = val;
//    fmtxunlk(&g_mem.priolk);

    return val;
}
#endif

struct memtls *
meminittls(void)
{
    struct memtls *tls = NULL;
    MEMPTR_T       adr;
    unsigned long  val;

    adr = mapanon(0, memtlssize());
    if (adr != MAP_FAILED) {
        tls = adr;
#if 0
        tls = (struct memtls *)memgenptrcl(adr, memtlssize(),
                                           sizeof(struct memtls));
#endif
#if (MEM_LK_TYPE & MEM_LK_PRIO)
        val = memgetprioval();
        priolkinit(&tls->priolkdata, val);
#endif
        pthread_once(&g_thronce, meminit);
        pthread_setspecific(g_thrkey, adr);
        g_memtls = tls;
    }

    return tls;
}

static void
memprefork(void)
{
    MEMUWORD_T slot;

//    fmtxlk(&g_mem.priolk);
    memgetlk(&g_mem.initlk);
    memgetlk(&g_mem.heaplk);
    for (slot = 0 ; slot < PTRBITS ; slot++) {
        memlkbit(&g_mem.smallbin[slot].list);
        memlkbit(&g_mem.bigbin[slot].list);
    }
    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
        memlkbit(&g_mem.pagebin[slot].list);
    }
#if (MEMMULTITAB)
    for (slot = 0 ; slot < MEMLVL1ITEMS ; slot++) {
        memlkbit(&g_mem.tab[slot].tab);
    }
#else
    for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
        memlkbit(&g_mem.hash[slot].chain);
    }
#endif

    return;
}

static void
mempostfork(void)
{
    MEMUWORD_T slot;

#if (MEMMULTITAB)
    for (slot = 0 ; slot < MEMLVL1ITEMS ; slot++) {
        memrelbit(&g_mem.tab[slot].tab);
    }
#else
    for (slot = 0 ; slot < MEMHASHITEMS ; slot++) {
        memrelbit(&g_mem.hash[slot].chain);
    }
#endif
    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
        memrelbit(&g_mem.pagebin[slot].list);
    }
    for (slot = 0 ; slot < PTRBITS ; slot++) {
        memrelbit(&g_mem.bigbin[slot].list);
        memrelbit(&g_mem.smallbin[slot].list);
    }
    memrellk(&g_mem.heaplk);
    memrellk(&g_mem.initlk);
//    fmtxunlk(&g_mem.priolk);

    return;
}

void
meminit(void)
{
    void       *heap;
    long        ofs;
    void       *ptr;
    MEMPTR_T   *adr;
    MEMUWORD_T  slot;

    memgetlk(&g_mem.initlk);
    pthread_atfork(memprefork, mempostfork, mempostfork);
    pthread_key_create(&g_thrkey, memfreetls);
#if (MEMSTAT)
    atexit(memprintstat);
#endif
    adr = mapanon(0, 3 * (2 * PTRBITS + MEMPAGESLOTS) * sizeof(long));
    if (adr == MAP_FAILED) {

        abort();
    }
    g_mem.bufvals.nblk[MEMSMALLBUF] = (MEMUWORD_T *)&adr[0];
    g_mem.bufvals.nblk[MEMPAGEBUF] = (MEMUWORD_T *)&adr[PTRBITS];
    g_mem.bufvals.nblk[MEMBIGBUF] = (MEMUWORD_T *)&adr[PTRBITS + MEMPAGESLOTS];
    g_mem.bufvals.ntls[MEMSMALLBUF] = (MEMUWORD_T *)&adr[2 * PTRBITS + MEMPAGESLOTS];
    g_mem.bufvals.ntls[MEMPAGEBUF] = (MEMUWORD_T *)&adr[3 * PTRBITS + MEMPAGESLOTS];
    g_mem.bufvals.ntls[MEMBIGBUF] = (MEMUWORD_T *)&adr[3 * PTRBITS + 2 * MEMPAGESLOTS];
    g_mem.bufvals.nglob[MEMSMALLBUF] = (MEMUWORD_T *)&adr[4 * PTRBITS + 2 * MEMPAGESLOTS];
    g_mem.bufvals.nglob[MEMPAGEBUF] = (MEMUWORD_T *)&adr[5 * PTRBITS + 2 * MEMPAGESLOTS];
    g_mem.bufvals.nglob[MEMBIGBUF] = (MEMUWORD_T *)&adr[5 * PTRBITS + 3 * MEMPAGESLOTS];
    for (slot = 0 ; slot < PTRBITS ; slot++) {
        g_mem.bufvals.nblk[MEMSMALLBUF][slot] = memnbufblk(MEMSMALLBUF, slot);
        g_mem.bufvals.nblk[MEMBIGBUF][slot] = memnbufblk(MEMSMALLBUF, slot);
        g_mem.bufvals.ntls[MEMSMALLBUF][slot] = memnbuftls(MEMSMALLBUF, slot);
//        g_mem.bufvals.ntls[MEMBIGBUF][slot] = memnbufblk(MEMSMALLBUF, slot);
        g_mem.bufvals.nglob[MEMSMALLBUF][slot] = memnbufglob(MEMSMALLBUF, slot);
        g_mem.bufvals.nglob[MEMBIGBUF][slot] = memnbufglob(MEMSMALLBUF, slot);
    }
    for (slot = 0 ; slot < MEMPAGESLOTS ; slot++) {
        g_mem.bufvals.nblk[MEMPAGEBUF][slot] = memnbufblk(MEMPAGEBUF, slot);
        g_mem.bufvals.ntls[MEMPAGEBUF][slot] = memnbuftls(MEMPAGEBUF, slot);
        g_mem.bufvals.nglob[MEMPAGEBUF][slot] = memnbufglob(MEMPAGEBUF, slot);
    }
#if (MEMARRAYHASH) || (MEMNEWHASH)
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

        abort();
    }
#if (MEMHASH) || (MEMARRAYHASH) || (MEMNEWHASH)
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
    memrellk(&g_mem.initlk);

    return;
}

MEMPTR_T
memputptr(struct membuf *buf, void *ptr, MEMUWORD_T size, MEMUWORD_T align)
{
    MEMUWORD_T type = memgetbuftype(buf);
    MEMUWORD_T slot = memgetbufslot(buf);
    MEMUWORD_T bsz = membufblksize(buf, type, slot);
    MEMUWORD_T id;
    MEMADR_T   adr = ptr;
    MEMADR_T   delta;

    if ((MEMADR_T)ptr & (align - 1)) {
        ptr = memalignptr(adr, align);
#if 0
        delta = (MEMADR_T)ptr - adr;
        bsz -= delta;
        if (bsz < size) {
            ptr = memgenptr(ptr, bsz, size);
        }
#endif
    }
    if (type != MEMPAGEBUF) {
        membufsetptr(buf, ptr, adr);
    } else {
        id = membufpageid(buf, ptr);
        membufsetpage(buf, id, adr);
    }

    return ptr;
}

static struct membuf *
memallocsmallbuf(MEMUWORD_T slot)
{
    MEMPTR_T       adr = SBRK_FAILED;
    MEMWORD_T      bufsz = memsmallbufsize(slot);
    MEMUWORD_T     nblk = MEMBUFBLKS;
    MEMUWORD_T     type = MEMSMALLBUF;
    MEMUWORD_T     info = 0;
    struct membuf *buf;

    if (!(g_mem.flg & MEMNOHEAPBIT)) {
        /* try to allocate from heap (sbrk()) */
        memgetlk(&g_mem.heaplk);
        adr = growheap(bufsz);
        if (adr != SBRK_FAILED) {
#if (MEMSTAT)
            g_memstat.nbheap += bufsz;
#endif
            info = MEMHEAPBIT;
        } else {
            g_mem.flg |= MEMNOHEAPBIT;
            memrellk(&g_mem.heaplk);
        }
    }
    if (adr == SBRK_FAILED) {
        /* sbrk() failed, let's try mmap() */
        adr = mapanon(0, bufsz);
        if (adr == MAP_FAILED) {
#if defined(ENOMEM)
            errno = ENOMEM;
#endif

            return NULL;
        }
#if (MEMSTAT)
        g_memstat.nbsmall += bufsz;
        g_memstat.nbmap += bufsz;
#endif
    }
    buf = (struct membuf *)adr;
#if (MEMBITFIELD)
    memsetbufflg(buf, 1);
#else
    buf->info = info;             // possible MEMHEAPBIT
#endif
    memsetbufslot(buf, slot);
    memsetbufnblk(buf, nblk);
    memsetbuftype(buf, type);
    buf->size = bufsz;
    buf->ptrtab = (MEMPTR_T *)((MEMPTR_T)buf + membufhdrsize());
#if (MEMSTAT)
    g_memstat.nbsmall += bufsz;
    g_memstat.nbheap += bufsz;
    g_memstat.nbbook += membufblkofs();
#endif
#if (MEMTEST)
    _memchkbuf(buf, slot, type, nblk, info, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitsmallbuf(struct membuf *buf, MEMUWORD_T slot,
                MEMUWORD_T size, MEMUWORD_T align)
{
    MEMUWORD_T     nblk = MEMBUFBLKS;
    MEMPTR_T       adr = (MEMPTR_T)buf;
    MEMPTR_T       ptr = adr + membufblkofs();
    struct membkt *dest = &g_memtls->smallbin[slot];
    MEMADR_T       upval;

    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
    ptr = memputptr(buf, ptr, size, align);
    memsetbuf(ptr, buf);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif
    /* FIXME */
#if 0
    memlkbit(&dest->list);
    upval = (MEMADR_T)dest->list;
    upval &= ~MEMLKBIT;
    buf->prev = NULL;
    buf->next = (struct membuf *)upval;
    if (upval) {
        ((struct membuf *)upval)->prev = buf;
    }
    buf->bkt = dest;
    m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)buf);
#endif

    return ptr;
}

static struct membuf *
memallocpagebuf(MEMUWORD_T slot, MEMUWORD_T nblk)
{
    MEMUWORD_T     mapsz = mempagebufsize(slot, nblk);
    MEMUWORD_T     type = MEMPAGEBUF;
    MEMPTR_T       adr;
    struct membuf *buf;

    /* mmap() blocks */
    adr = mapanon(0, mapsz);
    buf = (struct membuf *)adr;
#if (MEMBITFIELD)
    memclrbufflg(buf, 1);
#else
    buf->info = 0;
#endif
    memsetbufslot(buf, slot);
    memsetbufnblk(buf, nblk);
    memsetbuftype(buf, type);
    buf->size = mapsz;
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
#if (MEMSTAT)
    g_memstat.nbpage += mapsz;
    g_memstat.nbmap += mapsz;
    g_memstat.nbbook += membufblkofs();
#endif
    buf->ptrtab = (MEMPTR_T *)((MEMPTR_T)buf + membufhdrsize());
#if (MEMTEST)
    _memchkbuf(buf, slot, type, nblk, 0, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitpagebuf(struct membuf *buf, MEMUWORD_T slot,
               MEMUWORD_T size, MEMUWORD_T align,
               MEMUWORD_T nblk)
{
    MEMPTR_T       adr = (MEMPTR_T)buf;
    MEMPTR_T       ptr = adr + membufblkofs();
    struct membkt *dest = &g_memtls->pagebin[slot];
    MEMADR_T       upval;

    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
    ptr = memputptr(buf, ptr, size, align);
    memsetbuf(ptr, buf);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif
    /* FIXME */
#if 0
    if (nblk > 0) {
        memlkbit(&dest->list);
        upval = (MEMADR_T)dest->list;
        upval &= ~MEMLKBIT;
        buf->prev = NULL;
        buf->next = (struct membuf *)upval;
        if (upval) {
            ((struct membuf *)upval)->prev = buf;
        }
        buf->bkt = dest;
        m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)buf);
    }
#endif

    return ptr;
}

static struct membuf *
memallocbigbuf(MEMUWORD_T slot, MEMUWORD_T nblk)
{
    MEMUWORD_T     mapsz = membigbufsize(slot, nblk);
    MEMUWORD_T     type = MEMBIGBUF;
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
    memsetbuftype(buf, type);
#if (MEMSTAT)
    g_memstat.nbbig += mapsz;
    g_memstat.nbmap += mapsz;
    g_memstat.nbbook += membufblkofs();
#endif
    buf->size = mapsz;
    buf->ptrtab = (MEMPTR_T *)((MEMPTR_T)buf + membufhdrsize());
#if (MEMTEST)
    _memchkbuf(buf, slot, type, nblk, 0, __FUNCTION__);
#endif

    return buf;
}

static void *
meminitbigbuf(struct membuf *buf, MEMUWORD_T slot,
              MEMUWORD_T size, MEMUWORD_T align,
              MEMUWORD_T nblk)
{
    MEMPTR_T       adr = (MEMPTR_T)buf;
    MEMPTR_T       ptr = adr + membufblkofs();
    struct membkt *dest = &g_mem.bigbin[slot];
    MEMADR_T       upval;

    membufinitfree(buf, nblk);
    buf->base = ptr;
    nblk--;
    VALGRINDMKPOOL(ptr, 0, 0);
    memsetbufnfree(buf, nblk);
    ptr = memputptr(buf, ptr, size, align);
    memsetbuf(ptr, buf);
#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif
    /* FIXME */
#if 0
    if (nblk > 0) {
        memlkbit(&dest->list);
        upval = (MEMADR_T)dest->list;
        upval &= ~MEMLKBIT;
        buf->prev = NULL;
        buf->next = (struct membuf *)upval;
        if (upval) {
            ((struct membuf *)upval)->prev = buf;
        }
        buf->bkt = dest;
        m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t)buf);
    }
#endif

    return ptr;
}

void *
memgetbufblk(struct membuf *head, struct membkt *bkt,
             MEMUWORD_T size, MEMUWORD_T align)
{
    void          *ptr = NULL;
    MEMUWORD_T     nfree = memgetbufnfree(head);
    MEMUWORD_T     type = memgetbuftype(head);
    MEMUWORD_T     id = membufgetfree(head);

    nfree--;
    if (type != MEMPAGEBUF) {
        ptr = membufblkadr(head, id);
    } else {
        ptr = membufpageadr(head, id);
    }
    memsetbufnfree(head, nfree);
    ptr = memputptr(head, ptr, size, align);
    memsetbuf(ptr, head);
#if (MEMTEST)
    _memchkptr(head, ptr);
#endif
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

    return ptr;
}

#if (MEMNEWHASH)

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

static struct memhash *
memgethashitem(void)
{
    struct memhash *item = NULL;
    uint8_t        *first;
    struct memhash *prev;
    struct memhash *cur;
    uint8_t        *next;
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
        first = item;
        if (item == MAP_FAILED) {

            abort();
        }
        meminithashitem(first);
        upval = (MEMADR_T)g_mem.hashbuf;
        first += memhashsize();
        n--;
        next = first;
        meminithashitem(next);
        upval &= ~MEMLKBIT;
        while (--n) {
            prev = (struct memhash *)next;
            next += memhashsize();
            meminithashitem(next);
            prev->chain = (struct memhash *)next;
        }
        cur = (struct memhash *)next;
        cur->chain = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hashbuf, (m_atomic_t)first);
    }

    return item;
}

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

MEMADR_T
membufop(MEMPTR_T ptr, MEMWORD_T op, struct membuf *buf)
{
    MEMADR_T            adr = (MEMADR_T)ptr;
    MEMADR_T            page = adr >> PAGESIZELOG2;
    MEMUWORD_T          key = memhashptr(page) & (MEMHASHITEMS - 1);
    MEMADR_T            desc;
    MEMADR_T            val;
    MEMADR_T            upval;
    struct memhash     *blk;
    struct memhash     *prev;
    struct memhashitem *slot;
    struct memhashitem *src;
    MEMUWORD_T          lim;
    MEMUWORD_T          n;
    MEMUWORD_T          found;

    memlkbit(&g_mem.hash[key].chain);
    upval = (MEMADR_T)g_mem.hash[key].chain;
    upval &= ~MEMLKBIT;
    found = 0;
    blk = (struct memhash *)upval;
    while ((blk) && !found) {
        lim = blk->ntab;
        src = blk->tab;
        prev = NULL;
        do {
            n = min(lim, 4);
            switch (n) {
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
    upval = (MEMADR_T)g_mem.hash[key].chain;
    if (!found) {
        if (op == MEMHASHDEL || op == MEMHASHCHK) {
            memrelbit(&g_mem.hash[key].chain);

            return MEMHASHNOTFOUND;
        } else {
            desc = (MEMADR_T)buf;
            upval &= ~MEMLKBIT;
            blk = (struct memhash *)upval;
            if (blk) {
                prev = NULL;
                do {
                    n = blk->ntab;
                    if (n < MEMHASHITEMS) {
                        slot = blk->tab;
                        found++;
                        n++;
                        slot->nref = 1;
                        slot->nact = 1;
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
                            memrelbit(&g_mem.hash[key].chain);
#else
                            m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                                        (m_atomic_t)blk);
#endif
                        } else {
                            memrelbit(&g_mem.hash[key].chain);
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
                slot->nref = 1;
                slot->nact = 1;
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
                memrelbit(&g_mem.hash[key].chain);
            }
#if (MEMDEBUG)
            crash(desc != 0);
#endif
            
            return desc;
        }
    }
    desc = slot->val;
    upval &= ~MEMLKBIT;
    slot->nact++;
    if (op == MEMHASHDEL) {
        slot->nref--;
        if (!slot->nref) {
            if (blk->ntab == 1) {
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
                slot->nact = src->nact;
                upval &= ~MEMLKBIT;
                slot->page = src->page;
                slot->val = src->val;
                blk->ntab = n;
            }
        }
#if (MEMDEBUG)
        crash(desc != 0);
#endif
        memrelbit(&g_mem.hash[key].chain);

        return desc;
    } else if (op == MEMHASHADD) {
#if 0
        if (op == MEMHASHCHK) {
            desc = MEMHASHFOUND;
        }
#endif
        slot->nref += op;
    }
    upval &= ~MEMLKBIT;
    if (prev) {
        prev->chain = blk->chain;
        blk->chain = (struct memhash *)upval;
        m_syncwrite((m_atomic_t *)&g_mem.hash[key].chain,
                    (m_atomic_t)blk);
    } else {
        memrelbit(&g_mem.hash[key].chain);
    }
#if (MEMDEBUG)
    crash(desc != 0);
#endif
    
    return desc;
}

MEMPTR_T
memsetbuf(MEMPTR_T ptr, struct membuf *buf)
{
    MEMADR_T desc = membufop(ptr, MEMHASHADD, buf);

    return (MEMPTR_T)desc;
}

#endif

static MEMADR_T
memopenbuf(struct membkt *bkt)
{
    MEMADR_T upval;

    if (!bkt->list) {

        return 0;
    }
    memlkbit(&bkt->list);
    upval = (MEMADR_T)bkt->list;
    upval &= ~MEMLKBIT;
    if (!upval) {
        memrelbit(&bkt->list);
    }

    return upval;
}

MEMPTR_T
memtryblk(MEMUWORD_T slot, MEMUWORD_T type,
          MEMUWORD_T size, MEMUWORD_T align,
          struct membkt *bkt1, struct membkt *bkt2)
{
    MEMADR_T       upval = memopenbuf(bkt1);
    struct membuf *buf = (struct membuf *)upval;
    struct membkt *src = NULL;
    MEMPTR_T       ptr = NULL;
    MEMUWORD_T     info = 0;
    MEMUWORD_T     nblk = memgetnbufblk(slot, type);
    struct membkt *dest;
    
#if 0
    if (type == MEMSMALLBUF) {
        dest = &g_memtls->smallbin[slot];
    } else if (type == MEMPAGEBUF) {
        dest = &g_memtls->pagebin[slot];
    } else {
        dest = &g_mem.bigbin[slot];
    }
#endif
    if (buf) {
        src = bkt1;
        ptr = memgetbufblk(buf, bkt1, size, align);
    } else {
        if (bkt2) {
            upval = memopenbuf(bkt2);
            buf = (struct membuf *)upval;
        }
        if (buf) {
            src = bkt2;
            ptr = memgetbufblk(buf, bkt2, size, align);
        } else {
            if (type == MEMSMALLBUF) {
                buf = memallocsmallbuf(slot);
                if (buf) {
                    ptr = meminitsmallbuf(buf, slot, size, align);
                }
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
        }
    }
    if (!src && (ptr) && type == MEMSMALLBUF) {
#if (MEMBITFIELD)
        info = memgetbufflg(buf, 1);
#else
        info = buf->info;
#endif
        if (info & MEMHEAPBIT) {
            memlkbit(&g_mem.heap);
            upval = (MEMADR_T)g_mem.heap;
            upval &= ~MEMLKBIT;
            buf->heap = (struct membuf *)upval;
            /* this unlocks the global heap (low-bit becomes zero) */
            m_syncwrite(&g_mem.heap, buf);
            memrellk(&g_mem.heaplk);
        }
    }

    return ptr;
}

MEMPTR_T
memgetblk(MEMUWORD_T slot, MEMUWORD_T type, MEMUWORD_T size, MEMUWORD_T align)
{
    MEMPTR_T       ptr = NULL;
    MEMUWORD_T     info;
    MEMADR_T       upval = 0;
    
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
                        &g_mem.bigbin[slot], NULL);
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
memrelbuf(MEMUWORD_T slot, MEMUWORD_T type,
          struct membuf *buf, struct membkt *src)
{
    struct membkt *dest;
    MEMADR_T       upval;
    
    if (type == MEMSMALLBUF) {
        dest = &g_mem.smallbin[slot];
        if (src == &g_memtls->smallbin[slot]) {
            if (src->nbuf < memgetnbuftls(slot, MEMSMALLBUF)) {

                return;
            } else {
                if (buf->prev) {
                    buf->prev->next = buf->next;
                    memrelbit(&src->list);
                } else {
                    m_syncwrite((m_atomic_t *)&src->list,
                                (m_atomic_t)buf->next);
                }
                memlkbit(&dest->list);
                upval = (MEMADR_T)dest->list;
                upval &= ~MEMLKBIT;
                buf->prev = NULL;
                buf->next = (struct membuf *)upval;
                if (upval) {
                    ((struct membuf *)upval)->prev = buf;
                }
                dest->nbuf++;
                m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t *)buf);
            }

            return;
        } else if (src != &g_mem.smallbin[slot]) {
            memlkbit(&dest->list);
            upval = (MEMADR_T)dest->list;
            upval &= ~MEMLKBIT;
            buf->prev = NULL;
            buf->next = (struct membuf *)upval;
            if (upval) {
                ((struct membuf *)upval)->prev = buf;
            }
            dest->nbuf++;
            m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t *)buf);

            return;
        }
    } else if (type == MEMPAGEBUF) {
        dest = &g_mem.pagebin[slot];
        if (src == &g_memtls->pagebin[slot]) {
            if (src->nbuf < memgetnbuftls(slot, MEMPAGEBUF)) {

                return;
            } else {
                if (buf->prev) {
                    buf->prev->next = buf->next;
                    memrelbit(&src->list);
                } else {
                    m_syncwrite((m_atomic_t *)&src->list,
                                (m_atomic_t *)buf->next);
                }
                if (dest->nbuf < memgetnbufglob(slot, MEMPAGEBUF)) {
                    memlkbit(&dest->list);
                    upval = (MEMADR_T)dest->list;
                    upval &= ~MEMLKBIT;
                    buf->prev = NULL;
                    buf->next = (struct membuf *)upval;
                    if (upval) {
                        ((struct membuf *)upval)->prev = buf;
                    }
                    dest->nbuf++;
                    m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t *)buf);

                    return;
                }
            }
        }
        if (dest->nbuf >= memgetnbufglob(slot, MEMPAGEBUF)) {
            if (src == dest) {
                if (buf->prev) {
                    buf->prev->next = buf->next;
                    memrelbit(&src->list);
                } else {
                    m_syncwrite((m_atomic_t *)&src->list,
                                (m_atomic_t *)buf->next);
                }
            }
            unmapanon(buf->base, buf->size);
#if (MEMSTAT)
            g_memstat.nbpage -= buf->size;
            g_memstat.nbmap -= buf->size;
            g_memstat.nbbook -= membufblkofs();
#endif
        } else if (src != &g_mem.pagebin[slot]) {
            memlkbit(&dest->list);
            upval = (MEMADR_T)dest->list;
            upval &= ~MEMLKBIT;
            buf->prev = NULL;
            buf->next = (struct membuf *)upval;
            if (upval) {
                ((struct membuf *)upval)->prev = buf;
            }
            dest->nbuf++;
            m_syncwrite((m_atomic_t *)&dest->list, (m_atomic_t *)buf);
        }
    } else if (type == MEMBIGBUF) {
        if (src->nbuf >= memgetnbufglob(slot, MEMBIGBUF)) {
            if (buf->prev) {
                buf->prev->next = buf->next;
                memrelbit(&src->list);
            } else {
                m_syncwrite((m_atomic_t *)&src->list, (m_atomic_t *)buf->next);
            }
            unmapanon(buf, buf->size);
#if (MEMSTAT)
            g_memstat.nbmap -= buf->size;
            g_memstat.nbbig -= buf->size;
            g_memstat.nbbook -= membufblkofs();
#endif
        }
    }

    return;
}

void
memputblk(void *ptr, struct membuf *buf)
{
    struct membkt *bkt = buf->bkt;
    struct membkt *dest;
    MEMUWORD_T     slot = memgetbufslot(buf);
    MEMUWORD_T     nblk = memgetbufnblk(buf);
    MEMUWORD_T     nfree = memgetbufnfree(buf);
    MEMUWORD_T     type = memgetbuftype(buf);
    MEMUWORD_T     id;
    MEMADR_T       upval;

#if (MEMTEST)
    _memchkptr(buf, ptr);
#endif
    if (bkt) {
        memlkbit(&bkt->list);
    }
    nfree++;
    if (type != MEMPAGEBUF) {
        id = membufblkid(buf, ptr);
        membufsetptr(buf, ptr, NULL);
    } else {
        id = membufpageid(buf, ptr);
        membufsetpage(buf, id, NULL);
    }
    setbit(buf->freemap, id);
    memsetbufnfree(buf, nfree);
    if (nfree == nblk) {
        memrelbuf(slot, type, buf, bkt);
        if (bkt) {
            memrelbit(&bkt->list);
        }
    } else if (nfree == 1) {
        if (type == MEMSMALLBUF) {
            dest = &g_memtls->smallbin[slot];
            if (dest->nbuf >= memgetnbuftls(slot, MEMSMALLBUF)) {
                dest = &g_mem.smallbin[slot];
            }
        } else if (type == MEMPAGEBUF) {
            dest = &g_memtls->pagebin[slot];
            if (dest->nbuf >= memgetnbuftls(slot, MEMPAGEBUF)) {
                dest = &g_mem.pagebin[slot];
            }
        } else {
            dest = &g_mem.bigbin[slot];
        }
        memlkbit(&dest->list);
        upval = (MEMADR_T)dest->list;
        upval &= ~MEMLKBIT;
        buf->prev = NULL;
        buf->next = (struct membuf *)upval;
        if (upval) {
            ((struct membuf *)upval)->prev = buf;
        }
        buf->bkt = dest;
        dest->nbuf++;
        m_syncwrite((m_atomic_t *)&dest->list, buf);
    } else if (bkt) {
        memrelbit(&bkt->list);
    }

    return;
}

