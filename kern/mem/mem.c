#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#include <kern/mem/buf.h>
#include <kern/mem/pool.h>
#include <kern/mem/slab.h>
#if defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__)
#include <kern/mem/slab32.h>
#include <kern/unit/x86/link.h>
#elif defined(__arm__)
#include <kern/unit/arm/link.h>
#endif

extern struct mempool   memphyspool;
static struct membufbkt membufcputab[NCPU] ALIGNED(PAGESIZE);
static struct membufbkt membufbkt;

void
meminit(size_t nbphys)
{
    size_t lim = max(nbphys, KERNVIRTBASE);

#if (defined(__i386__) && !defined(__x86_64__) && !defined(__amd64__))  \
    || defined(__arm__)
    vminitvirt(&_pagetab, &_epagetab,
              lim - (uint32_t)&_epagetab,
              PAGEWRITE);
    meminitphys(&memphyspool, (size_t)&_epagetab,
                lim - (size_t)&_epagetab);
#elif defined(__x86_64__) || defined(__amd64__)
#error implement x86-64 memory management
#endif

//    swapinit(0, 0x00000000, 1024);

    return;
}

void
meminitphys(struct mempool *physpool, uintptr_t base, size_t nbphys)
{
    struct memslab **blktab = (struct memslab **)physpool->tab;
    uintptr_t        adr = ((base & (MEMMIN - 1))
                            ? rounduppow2(base, MEMMIN)
                            : base);
    unsigned long    bkt = PTRBITS - 1;
    size_t           sz = 1UL << bkt;
    struct memslab  *slab;

    adr = meminitpool(physpool, adr, nbphys);
    nbphys -= adr - base;
    nbphys = rounddownpow2(nbphys, MEMMIN);
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + nbphys,
             PAGEWRITE);
#if (__KERNEL__)
    kprintf("%ld kilobytes kernel virtual memory free @ 0x%lx\n",
            nbphys >> 10, adr);
#endif
    while ((nbphys) && bkt >= MEMMINLOG2) {
        if (nbphys & sz) {
            slab = memgethdr(adr, physpool);
            memslabclrinfo(slab);
            memslabclrlink(slab);
            memslabsetbkt(slab, bkt);
            memslabsetfree(slab);
            blktab[bkt] = slab;
            nbphys &= ~sz;
            adr += sz;
        }
        bkt--;
        sz >>= 1;
    }
#if (__KERNEL__ && (MEMDIAG))
    memdiag(physpool);
#endif

    return;
}

/*
 * called without locks at boot time, or with locks held by memgetbuf() */
long
meminitcpubuf(long unit, long how)
{
    struct membufbkt *bkt = &membufcputab[unit];
    uint8_t          *u8ptr = kwalloc(PAGESIZE);
    long              n = PAGESIZE / MEMBUF_SIZE;
    struct membuf    *buf;
    void             *last;

    if (!u8ptr) {

        return 0;
    }
    u8ptr += PAGESIZE;
    bkt->nbuf = n;
    while (n--) {
        u8ptr -= MEMBUF_SIZE;
        buf = (struct membuf *)u8ptr;
        buf->hdr.next = last;
        last = buf;
    }
    bkt->buflist = last;

    return 1;
}

long
meminitbuf(void)
{
    struct membuf *buf;
    struct memblk *blk;
    void          *last;
    uint8_t       *u8ptr1;
    uint8_t       *u8ptr2;
    long           n;

    /* allocate wired memory for membufs and memblks */
    u8ptr1 = kwalloc(MEMNBUF * MEMBUF_SIZE);
    if (!u8ptr1) {
        kprintf("FAILED to allocate membufs\n");
        m_waitint();
    }
    kbzero(u8ptr1, MEMNBUF * MEMBUF_SIZE);
    u8ptr2 = kwalloc(NMEMBUFBLK * MEMBUF_BLK_SIZE);
    if (!u8ptr2) {
        kprintf("FAILED to allocate blks for membufs\n");
        m_waitint();
    }
    /* zero all allocated memory */
    kbzero(u8ptr2, NMEMBUFBLK * MEMBUF_BLK_SIZE);
    /* allocate global buf container */
    n = MEMNBUF;
    membufbkt.nbuf = n;
    u8ptr1 += MEMNBUF * MEMBUF_SIZE;
    last = NULL;
    while (n--) {
        u8ptr1 -= MEMBUF_SIZE;
        buf = (struct membuf *)u8ptr1;
        buf->hdr.next = last;
        last = buf;
    }
    membufbkt.buflist = last;
    /* allocate global blk container */
    n = NMEMBUFBLK;
    membufbkt.nblk = n;
    u8ptr2 += NMEMBUFBLK * MEMBUF_BLK_SIZE;
    last = NULL;
    while (n--) {
        u8ptr2 -= MEMBUF_BLK_SIZE;
        blk = (struct memblk *)u8ptr1;
        blk->ptr = blk;
        blk->next = last;
        last = blk;
    }
    membufbkt.blklist = last;
    /* initialise per-CPU buf containers */
    n = NCPU;
    while (n--) {
        meminitcpubuf(n, MEM_DONTWAIT);
    }
}

/* FIXME: steal mbufs from other CPUs if need arises */
struct membuf *
memgetbuf(long how)
{
    volatile long     unit = k_curunit;
    uint8_t          *ptr;
    struct membufbkt *bkt;
    struct membuf    *ret = NULL;
    struct membuf    *buf;
    struct membuf    *last;
    long              n;

    mtxlk(&membufcputab[unit].lk);
    ret = membufcputab[unit].buflist;
    if (ret) {
        membufcputab[unit].buflist = buf->hdr.next;
        membufcputab[unit].nbuf--;
    } else {
        mtxlk(&membufbkt.lk);
        ret = membufbkt.buflist;
        if (ret) {
            membufbkt.buflist = buf->hdr.next;
            membufbkt.nbuf--;
            mtxunlk(&membufbkt.lk);
        } else {
            mtxunlk(&membufbkt.lk);
            meminitcpubuf(unit, MEM_TRYWAIT);
        }
    }
    mtunxlk(&membufcputab[unit].lk);

    return ret;
}

void
memputbuf(struct membuf *buf)
{
    struct membkt *bkt = &membufbkt;

    mtxlk(&bkt->lk);
    buf->hdr.next = membufbkt.buflist;
    membufbkt.buflist = buf;
    membufbkt.nbuf++;
    mtxunlk(&bkt->lk);
}

/* FIXME: steal mbufs from other CPUs if need arises */
void *
memgetblk(long how)
{
    volatile long     unit = k_curunit;
    uint8_t          *ptr;
    struct membufbkt *bkt;
    struct memblk    *ret = NULL;
    struct memblk    *blk;
    struct memblk    *last;
    long              n;

    mtxlk(&membufcputab[unit].lk);
    ret = membufcputab[unit].blklist;
    if (ret) {
        membufcputab[unit].blklist = blk->next;
        membufcputab[unit].nblk--;
    } else {
        mtxlk(&membufbkt.lk);
        ret = membufbkt.blklist;
        if (ret) {
            membufbkt.blklist = blk->next;
            membufbkt.nblk--;
        }
        mtxunlk(&membufbkt.lk);
    }
    mtunxlk(&membufcputab[unit].lk);

    return ret;
}

void
memputblk(struct memblk *blk)
{
    struct membkt *bkt = &membufbkt;

    mtxlk(&bkt->lk);
    blk->next = membufbkt.blklist;
    membufbkt.blklist = blk;
    membufbkt.nblk++;
    mtxunlk(&bkt->lk);
}

struct membuf *
membufprepend(struct membuf *buf, size_t len, long how)
{
    struct membuf *mb;

    if (buf->hdr.flg & MEMBUF_PKTHDR_BIT) {
        _memgetpkthdr(mb, how, mb->hdr.type);
    } else {
        _memgetbuf(mb, how, mb->hdr.type);
    }
    if (!mb) {
        while (buf) {
            buf = memfreebuf(buf);
        }

        return NULL;
    }
    if (buf->hdr.flg & MEMBUF_PKTHDR_BIT) {
        memmovepkthdr(mb, buf);
    }
    mb->hdr.next = buf;
    buf = mb;
    if (len < membuflen(buf)) {
        _membufalign(buf, len);
    }
    buf->hdr.len = len;

    return buf;
}

