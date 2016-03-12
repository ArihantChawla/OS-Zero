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
#include <kern/mem/buf.h>
#include <kern/mem/pool.h>
#include <kern/mem/mag.h>
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__) && 0
#include <kern/mem/slab32.h>
#include <kern/unit/x86/link.h>
#elif defined(__arm__)
#include <kern/unit/arm/link.h>
#endif

extern struct mempool   memphyspool;
extern struct mempool   memvirtpool;
static struct membufbkt membufbkttab[NCPU] ALIGNED(PAGESIZE);
static struct membufbkt membufbkt;

void
meminitvirtpool(struct mempool *pool, uintptr_t base, size_t nbyte)
{
    uintptr_t       adr = ((base & (MEMMINSIZE - 1))
                           ? rounduppow2(base, MEMMINSIZE)
                           : base);
    unsigned long   bktid = PTRBITS - 1;
    struct membkt   *tab = pool->tab;
    size_t          sz = 1UL << bktid;
    struct memmag  *mag;
    
    adr = meminitpool(pool, adr, nbyte);
    nbyte -= adr - base;
    nbyte = rounddownpow2(nbyte, PAGESIZE);
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + nbyte,
             PAGEWRITE);
#if (__KERNEL__)
    kprintf("%ld kilobytes kernel virtual memory free @ 0x%lx\n",
            nbyte >> 10, adr);
#endif
    while ((nbyte) && bktid >= PAGESIZELOG2) {
        if (nbyte & sz) {
            mag = memgetmag(adr, pool);
            memmagclrinfo(mag);
            memmagclrlink(mag);
            memmagsetbkt(mag, bktid);
            memmagsetfree(mag);
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

#if 0
void
meminitphyspool(struct mempool *pool, uintptr_t base, size_t nbyte)
{
    uintptr_t       adr = ((base & (MEMMINSIZE - 1))
                           ? rounduppow2(base, MEMMINSIZE)
                           : base);
    unsigned long   bktid = PTRBITS - 1;
    struct membkt  *tab = pool->tab;
    size_t          sz = PAGESIZE;
    size_t          n;
    struct memmag *mag;
    
    adr = meminitpool(pool, adr, nbyte);
    nbyte -= adr - base;
    nbyte = rounddownpow2(nbyte, PAGESIZE);
    n = nbyte >> PAGESIZELOG2;
    if (map) {
        vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + nbyte,
                 PAGEWRITE);
    }
#if (__KERNEL__)
    kprintf("%ld kilobytes kernel physical memory free @ 0x%lx\n",
            nbyte >> 10, adr);
#endif
    while (n--) {
    }
#if 0
    while ((nbyte) && bktid >= PAGESIZELOG2) {
        if (nbyte & sz) {
            mag = memgetmag(adr, pool);
            memmagclrinfo(mag);
            memmagclrlink(mag);
            memmagsetbkt(mag, bktid);
            memmagsetfree(mag);
            tab[bktid].list = mag;
            nbyte -= sz;
            adr += sz;
        }
        bktid--;
        sz >>= 1;
    }
#endif
#if (__KERNEL__ && defined(MEMDIAG)) && 0
    memdiag(pool);
#endif
    
    return;
}
#endif

/*
 * called without locks at boot time, or with locks held by memgetbuf() */
long
meminitcpubuf(long unit, long how)
{
    struct membufbkt *tab = &membufbkttab[unit];
    uint8_t          *u8ptr = kwalloc(PAGESIZE);
    long              n = PAGESIZE / MEMBUF_SIZE;
    void             *last = NULL;
    struct membuf    *buf;

    if (!u8ptr) {

        return 0;
    }
    u8ptr += PAGESIZE;
    tab->nbuf = n;
    while (n--) {
        u8ptr -= MEMBUF_SIZE;
        buf = (struct membuf *)u8ptr;
        buf->hdr.next = last;
        last = buf;
    }
    tab->buflist = last;

    return 1;
}

long
meminitbuf(void)
{
    long              unit = k_curunit;
    struct membufbkt *tab = &membufbkttab[unit];
    struct membuf    *buf;
    struct memblk    *blk;
    void             *last;
    uint8_t          *u8ptr1;
    uint8_t          *u8ptr2;
    long              n;

    /* allocate wired memory for membufs */
    u8ptr1 = kwalloc(MEMNBUF * MEMBUF_SIZE);
    if (!u8ptr1) {
        kprintf("FAILED to allocate membufs\n");
        m_waitint();
    }
    kbzero(u8ptr1, MEMNBUF * MEMBUF_SIZE);
    /* initialise global membuf container */
    n = MEMNBUF;
    tab->nbuf = n;
    u8ptr1 += MEMNBUF * MEMBUF_SIZE;
    last = NULL;
    while (n--) {
        u8ptr1 -= MEMBUF_SIZE;
        buf = (struct membuf *)u8ptr1;
        buf->hdr.next = last;
        last = buf;
    }
    tab->buflist = last;
    /* initialise per-CPU buf containers */
    n = NCPU;
    while (n--) {
        meminitcpubuf(n, MEM_DONTWAIT);
    }
}

void
meminit(size_t nbphys, size_t nbvirt)
{
    size_t    lim = max(nbphys, KERNVIRTBASE);
    uintptr_t adr = (uintptr_t)&_ebss;

#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
    pageinitphys((uintptr_t)&_epagetab, lim - (size_t)&_epagetab);
    lim = max(nbvirt, KERNVIRTBASE);
    meminitvirtpool(&memvirtpool, (uintptr_t)&_eusr,
                    lim - (size_t)&_eusr);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif
//    meminitbuf();
//    swapinit(0, 0x00000000, 1024);
    
    return;
}

/* FIXME: steal mbufs from other CPUs if need arises */
struct membuf *
memgetbuf(long how)
{
    volatile long     unit = k_curunit;
    uint8_t          *ptr;
    struct membufbkt *tab = &membufbkttab[unit];
    struct membuf    *ret = NULL;
    struct membuf    *buf;
    struct membuf    *last;
    long              n;

    mtxlk(&tab->lk);
    ret = tab->buflist;
    if (ret) {
        tab->buflist = buf->hdr.next;
        tab->nbuf--;
    }
    mtxunlk(&tab->lk);

    return ret;
}

void
memputbuf(struct membuf *buf)
{
    long              unit = k_curunit;
    struct membufbkt *tab = &membufbkttab[unit];

    mtxlk(&tab->lk);
    buf->hdr.next = tab->buflist;
    tab->buflist = buf;
    tab->nbuf++;
    mtxunlk(&tab->lk);

    return;
}

#if 0
/* FIXME: steal mbufs from other CPUs if need arises */
void *
memgetblk(long how)
{
    volatile long     unit = k_curunit;
    struct membufbkt *tab = &membufbkttab[unit];
    uint8_t          *ptr;
    struct memblk    *ret = NULL;
    struct memblk    *blk;
    struct memblk    *last;
    long              n;

    mtxlk(&tab->lk);
    ret = tab->buflist;
    if (ret) {
        tab->buflist = blk->next;
        tab->nblk--;
    } else {
        mtxlk(&tab->lk);
        ret = tab->buflist;
        if (ret) {
            tab->buflist = blk->next;
            tab->nblk--;
        }
        mtxunlk(&tab->lk);
    }
    mtxunlk(&tab->lk);

    return ret;
}

void
memputblk(struct memblk *blk)
{
    long              unit = k_curunit;
    struct membufbkt *tab = &membufbkttab[unit];

    mtxlk(&tab->lk);
    blk->next = tab->buflist;
    tab->buflist = blk;
    tab->nblk++;
    mtxunlk(&tab->lk);
}
#endif

#if 0

struct membuf *
membufprepend(struct membuf *buf, size_t len, long how)
{
    struct membuf *mb = (buf);

    if (buf->hdr.flg & MEMBUF_PKTHDR_BIT) {
        _memgetpkthdr(mb, how, mb->hdr.type);
    } else {
        _memgetbuf(mb, how, mb->hdr.type);
    }
    if (!mb) {

        return NULL;
    }
    if (buf->hdr.flg & MEMBUF_PKTHDR_BIT) {
        _memmovepkthdr(mb, buf);
    }
    mb->hdr.next = buf;
    buf = mb;
    if (len < membuflen(buf)) {
        buf->hdr.adr = _memalignbuf(buf, len);
    }
    buf->hdr.len = len;

    return buf;
}

#endif

