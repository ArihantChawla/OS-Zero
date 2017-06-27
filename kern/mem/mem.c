#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zero/mtx.h>
#include <kern/util.h>
#include <kern/cpu.h>
#include <kern/malloc.h>
#include <kern/mem/vm.h>
#include <kern/mem/page.h>
#include <kern/mem/membuf.h>
#include <kern/mem/pool.h>
#include <kern/mem/mag.h>
#include <kern/mem/slab.h>
#if defined(__arm__)
#include <kern/unit/arm/link.h>
#endif

extern struct kmempool   kmemphyspool;
extern struct kmempool   kmemvirtpool;
//static struct kmembufbkt kmembufbkttab[NCPU] ALIGNED(PAGESIZE);
static struct kmembufbkt kmembufbkt;

void
meminitphys(struct kmempool *pool, uintptr_t base, size_t nbyte)
{
    uintptr_t       adr = base;
//    unsigned long sz = (nbyte & (MEMSLABSIZE - 1)) ? rounddownpow2(nbyte, MEMSLABSIZE) : nbyte;
    size_t           sz = nbyte;
    intptr_t         ofs = base & (MEMSLABSIZE - 1);
    struct kmemslab *slab;
    struct kmemmag  *mag;
    size_t           nblk;
    size_t           hdrsz;

    if (ofs) {
        adr += MEMSLABSIZE - ofs;
        sz -= adr - base;
    }
    nblk = sz >> MEMMINSHIFT;
    /* configure slab headers */
    hdrsz = nblk * sizeof(struct kmemslab);
    hdrsz = rounduppow2(hdrsz, MEMSLABSIZE);
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE | PAGEWIRED);
//    kbzero((void *)adr, hdrsz);
    pool->nblk = nblk;
    pool->hdrtab = (void *)adr;
#if (__KERNEL__)
    kprintf("MEM: reserved %lu bytes @%lx for %lu slab headers\n",
            hdrsz, adr, nblk);
#endif
    adr += hdrsz;
    /* configure magazine headers */
    hdrsz = nblk * sizeof(struct kmemmag);
    hdrsz = rounduppow2(hdrsz, MEMSLABSIZE);
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE | PAGEWIRED);
//    kbzero((void *)adr, hdrsz);
    kmemvirtpool.nblk = nblk;
    kmemvirtpool.hdrtab = (void *)adr;
#if (__KERNEL__)
    kprintf("MEM: reserved %lu bytes @%lx for %lu magazine headers\n",
            hdrsz, adr, nblk);
#endif
    adr += hdrsz;
    pool->base = adr;
    kmemvirtpool.base = adr;
#if (__KERNEL__ && defined(MEMDIAG) && (MEMDIAG))
    memdiag(kmemvirtpool);
#endif

    return;
}

void
meminitvirt(struct kmempool *pool, size_t lim)
{
    uintptr_t       adr = pool->base;
    unsigned long   bktid = PTRBITS - 1;
    struct kmembkt *tab = pool->tab;
    size_t          sz = 1UL << bktid;
    size_t          nbyte = lim - adr;
    uintptr_t       orig = adr;
    struct kmemmag *mag;

    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + nbyte,
             PAGEWRITE);
//    kbzero((void *)adr, nbyte);
#if (__KERNEL__)
    kprintf("%ld kilobytes kernel virtual memory free @ 0x%lx\n",
            nbyte >> 10, adr);
#endif
    while ((nbyte) && bktid >= PAGESIZELOG2) {
        if (nbyte & sz) {
            mag = memgetmag(adr, pool);
            memclrmaginfo(mag);
            memclrmaglink(mag);
            memsetmagbkt(mag, bktid);
            memsetmagfree(mag);
            tab[bktid].list = mag;
            nbyte -= sz;
            adr += sz;
        }
        bktid--;
        sz >>= 1;
    }
#if (__KERNEL__ && defined(MEMDIAG)) && 0
    memdiag(pool);
#endif
    
    return;
}

void
meminit(size_t nbphys, size_t nbvirt)
{
    size_t    lim = max(nbphys, KERNVIRTBASE);
    uintptr_t adr;

    vmmapseg(&_pagetab,
             (uint32_t)nbvirt,
             (uint32_t)nbvirt,
             (uint32_t)lim,
             PAGEPRES | PAGEWRITE);
#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
    pageinitphys((uintptr_t)&_epagetab, lim - (size_t)&_epagetab);
    meminitphys(&kmemphyspool, (uintptr_t)&_epagetab,
                lim - (size_t)&_epagetab);
    lim = max(nbvirt, KERNVIRTBASE);
    meminitvirt(&kmemvirtpool,
                lim);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif
//    meminitbuf();
//    swapinit(0, 0x00000000, 1024);
    
    return;
}

#if 0
/*
 * called without locks at boot time, or with locks held by memgetbuf() */
long
meminitcpubuf(long unit, long how)
{
    struct kmembufbkt *tab = &kmembufbkttab[unit];
    uint8_t           *u8ptr = kwalloc(PAGESIZE);
    long               n = PAGESIZE / MEMBUF_SIZE;
    void              *last = NULL;
    struct kmembuf    *buf;

    if (!u8ptr) {

        return 0;
    }
    u8ptr += PAGESIZE;
    tab->nbuf = n;
    while (n--) {
        u8ptr -= MEMBUF_SIZE;
        buf = (struct kmembuf *)u8ptr;
        buf->hdr.next = last;
        last = buf;
    }
    tab->buflist = last;

    return 1;
}
#endif

long
meminitbuf(void)
{
    struct kmembufbkt *tab = &kmembufbkt;
    struct kmembuf    *buf;
    struct memblk     *blk;
    void              *last;
    uint8_t           *u8ptr1;
    uint8_t           *u8ptr2;
    long               n;

    /* allocate wired memory for membufs */
    u8ptr1 = kwalloc(MEMNBUF * MEMBUF_SIZE);
    if (!u8ptr1) {
        kprintf("FAILED to allocate membufs\n");

        return 0;
    }
    kbzero(u8ptr1, MEMNBUF * MEMBUF_SIZE);
    /* initialise global membuf container */
    n = MEMNBUF;
    tab->nbuf = n;
    u8ptr1 += MEMNBUF * MEMBUF_SIZE;
    last = NULL;
    while (n--) {
        u8ptr1 -= MEMBUF_SIZE;
        buf = (struct kmembuf *)u8ptr1;
        buf->hdr.next = last;
        last = buf;
    }
    tab->buflist = last;
#if 0
    /* initialise per-CPU buf containers */
    n = NCPU;
    while (n--) {
        meminitcpubuf(n, MEM_DONTWAIT);
    }
#endif

    return 1;
}

/* FIXME: steal membufs from other CPUs if need arises */
struct kmembuf *
memallocbuf(long how)
{
    struct kmembufbkt *tab = &kmembufbkt;
    struct kmembuf    *ret = NULL;
    uint8_t           *ptr;
    struct kmembuf    *buf;
    struct kmembuf    *last;
    long               loop;
    long               n;

    loop = how & MEM_WAIT;
    do {
        fmtxlk(&tab->lk);
        ret = tab->buflist;
        if (ret) {
            tab->buflist = ret->hdr.next;
            tab->nbuf--;
        }
        fmtxunlk(&tab->lk);
    } while (!ret && (loop));

    return ret;
}

void
memfreebuf(struct kmembuf *buf)
{
    struct kmembufbkt *tab = &kmembufbkt;

    fmtxlk(&tab->lk);
    buf->hdr.next = tab->buflist;
    tab->buflist = buf;
    tab->nbuf++;
    fmtxunlk(&tab->lk);

    return;
}

#if 0

/* FIXME: steal mbufs from other CPUs if need arises */
void *
memallocblk(long how)
{
    volatile long      unit = k_curunit;
    struct kmembufbkt *tab = &kmembufbkttab[unit];
    uint8_t           *ptr;
    struct memblk     *ret = NULL;
    struct memblk     *blk;
    struct memblk     *last;
    long               loop;
    long               n;

    loop = how & MEM_WAIT;
    do {
        fmtxlk(&tab->lk);
        ret = tab->buflist;
        if (ret) {
            tab->buflist = blk->next;
            tab->nblk--;
        } else {
            fmtxlk(&tab->lk);
            ret = tab->buflist;
            if (ret) {
                tab->buflist = blk->next;
                tab->nblk--;
            }
            fmtxunlk(&tab->lk);
        }
        fmtxunlk(&tab->lk);
    } while (!ptr && (loop));

    return ret;
}

void
memfreeblk(struct memblk *blk)
{
    long               unit = k_curunit;
    struct kmembufbkt *tab = &kmembufbkttab[unit];

    fmtxlk(&tab->lk);
    blk->next = tab->buflist;
    tab->buflist = blk;
    tab->nblk++;
    fmtxunlk(&tab->lk);
}

#endif

