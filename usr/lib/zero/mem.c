#include <stddef.h>
#include <stdint.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/unix.h>
#include <zero/mem.h>

typedef struct membuf * memallocbuffunc(struct mem *mem, long slot);
typedef void * meminitbuffunc(struct mem *mem, struct membuf *buf);

static THREADLOCAL struct memarn     *tls_arn ALIGNED(PAGESIZE);
#if (MEM_LK_TYPE == MEM_LK_PRIO)
static THREADLOCAL struct priolkdata  tls_priolkdata;
#endif
static THREADLOCAL pthread_once_t     tls_once;
static THREADLOCAL pthread_key_t      tls_key;
static THREADLOCAL long               tls_flg;
static struct                         mem g_mem ALIGNED(CLSIZE);

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
    buf->slot = slot;             // slot #

    return buf;
}

static void *
meminitsmallbuf(struct mem *mem, struct membuf *buf)
{
    MEMWORD_T  nblk = MEMBUFFREEMAPWORDS * CHAR_BIT * sizeof(MEMWORD_T);
    long       slot = buf->slot;
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
    if (!info) {
        /* link block from sbrk() to global heap (put it on top) */
        bptr = mem->heap;
        buf->heap = bptr;
    }

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
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    buf->slot = slot;           // slot #

    return buf;
}

static void *
meminitpagebuf(struct mem *mem, struct membuf *buf)
{
    MEMWORD_T nblk = MEMBUFFREEMAPWORDS * CHAR_BIT * sizeof(MEMWORD_T);
    long      slot = buf->slot;
    MEMPTR_T  adr = (MEMPTR_T)buf;
    MEMWORD_T blksz = slot * PAGESIZE;
    MEMPTR_T  ptr = adr + membufblkofs();

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;

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
    if (adr == MAP_FAILED) {
        
        return NULL;
    }
    buf->slot = slot;           // slot #

    return buf;
}

static void *
meminitbigbuf(struct mem *mem, struct membuf *buf, MEMUWORD_T nblk)
{
    long       slot = buf->slot;
    MEMPTR_T   adr = (MEMPTR_T)buf;
    MEMUWORD_T blksz = MEMWORD(1) << (slot);
    MEMPTR_T   ptr = adr + membufblkofs();

    /* set number of blocks for buf */
    memsetbufnblk(buf, nblk);
    /* initialise freemap */
    membufinitfree(buf, nblk);
    buf->base = ptr;

    return ptr;
}

struct membuf *
membufgetblk(struct membuf *head, MEMPTR_T *retptr)
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
        prev->prev = NULL;
        prev->next = NULL;
    }
    if (head) {
        *retptr = membufblkadr(head, ndx);
    }

    return head;
}

/* FIXME */
static void *
memgetblk(struct mem *mem, long slot, long type)
{
    struct memarn *arn = tls_arn;
    MEMPTR_T       ptr = NULL;
    struct membkt *bkt;
    struct membuf *buf;
    MEMUWORD_T     info;
    MEMUWORD_T     nblk = 1;
    struct membuf *bptr;
    MEMADR_T       upval;

    if (!type) {
        bkt = &arn->smallbin[slot];
        memlkbit((volatile long *)&bkt->list);
        upval = (MEMADR_T)bkt->list;
        upval &= ~MEMLKBIT;
        buf = (struct membuf *)upval;
        if (upval) {
            /* TODO */
            upval = (MEMADR_T)membufgetblk(buf, &ptr);
            m_syncwrite((volatile long *)&bkt->list, upval);
        } else {
            /* TODO: try global buffer */
            buf = memallocsmallbuf(mem, slot);
            if (buf) {
                ptr = meminitsmallbuf(mem, buf);
            }
        }
    } else if (type == 1) {
        bkt = &arn->pagebin[slot];
        memlkbit((volatile long *)&bkt->list);
        upval = (MEMADR_T)bkt->list;
        upval &= ~MEMLKBIT;
        buf = (struct membuf *)upval;
        if (upval) {
            upval = (MEMADR_T)membufgetpages(buf, &ptr);
            m_syncwrite((volatile long *)&bkt->list, upval);
        } else {
            /* TODO: try global buffer */
            buf = memallocpagebuf(mem, slot, nblk);
            if (buf) {
                ptr = meminitpagebuf(mem, buf);
            }
        }
    } else {
        bkt = &g_mem.bigbin[slot];
        memlkbit((volatile long *)&bkt->list);
        upval = (MEMADR_T)bkt->list;
        upval &= ~MEMLKBIT;
        buf = (struct membuf *)upval;
        if (upval) {
            upval = (MEMADR_T)membufgetblk(buf, &ptr);
            m_syncwrite((volatile long *)&bkt->list, upval);
        } else {
            /* TODO: try global buffer */
            buf = memallocbigbuf(mem, slot, nblk);
            if (buf) {
                ptr = meminitbigbuf(mem, buf, nblk);
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
        buf->ptrtab = (MEMPTR_T)buf + membufatabofs();
        if (info & MEMHEAPBIT) {
            /* this unlocks the global heap (low-bit becomes zero) */
            m_syncwrite(&mem->heap, buf);
        }
        /* this unlocks the arena bucket (low-bit becomes zero) */
        m_syncwrite((volatile long *)&bkt->list, buf);
    } else {
        memrelbit((volatile long *)&bkt->list);
    }

    return ptr;                 // return pointer to first block
}

/* find a buf address; type encoded in the low 2 bits */
static void *
memputbkt(void *ptr, struct membkt *bkt, long type)
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
    ((MEMADR_T *)itab)[k3] = (MEMADR_T)bkt | type;
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
    ((MEMADR_T *)itab)[k4] = (MEMADR_T)bkt | type;
#endif
    memrellk(&g_mem.tab[k1].lk);
    
    return ptr;
}

static MEMADR_T
memfindbkt(void *ptr)
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

    return ret;
}

