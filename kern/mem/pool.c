#include <stddef.h>
#include <stdint.h>
#if (MEMTEST)
#include <stdlib.h>
#endif
#include <errno.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zero/mtx.h>
#include <kern/util.h>
#include <kern/cpu.h>
#include <kern/mem/mem.h>
#include <kern/mem/slab.h>
#include <kern/mem/mag.h>
#include <kern/mem/bkt.h>
#include <kern/proc/proc.h>
#if defined(__i386__)
#include <kern/unit/x86/link.h>
#include <kern/unit/x86/trap.h>
#endif
#include <kern/mem/mem.h>
#if (MEMTEST)
#include <stdio.h>
#include <string.h>
#define kprintf printf
#define kbzero bzero
#define panic(pid, trap, err) abort()
#endif

extern struct mempool memphyspool;
struct mempool        memvirtpool;

void *
memalloc(size_t nb, long flg)
{
    struct mempool  *physpool = &memphyspool;
    struct mempool  *virtpool = &memvirtpool;
    struct memmag  **magtab = (struct maghdr **)virtpool->tab;
    void            *ptr = NULL;
    size_t           sz = max(MEMMIN, nb);
    size_t           bsz;
    unsigned long    slab = 0;
    unsigned long    bkt = memcalcbkt(sz);
#if defined(MEMPARANOIA)
    unsigned long   *bmap;
#endif
    struct memmag   *mag;
    uint8_t         *u8ptr;
    unsigned long    ndx;
    unsigned long    n;
    struct membkt   *hdr = &virtpool->tab[bkt];

    mtxlk(&hdr->lk);
    if (bkt >= MEMSLABMINLOG2) {
        ptr = slaballoc(physpool, sz, flg);
        if (ptr) {
#if (!MEMTEST)
            vminitvirt(&_pagetab, ptr, sz, flg);
#endif
            slab++;
            mag = memgetmag(ptr, virtpool);
            mag->base = (uintptr_t)ptr;
            mag->n = 1;
            mag->ndx = 1;
            mag->bkt = bkt;
            mag->prev = NULL;
            mag->next = NULL;
        }
    } else {
        mag = magtab[bkt];
        if (mag) {
            ptr = mempop(mag);
            if (memmagempty(mag)) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                magtab[bkt] = mag->next;
            }
        } else {
            ptr = slaballoc(physpool, sz, flg);
            if (ptr) {
#if (!MEMTEST)
                vminitvirt(&_pagetab, ptr, sz, flg);
#endif
                u8ptr = ptr;
                slab++;
                bsz = (uintptr_t)1 << bkt;
                n = (uintptr_t)1 << (MEMSLABMINLOG2 - bkt);
                mag = memgetmag(ptr, virtpool);
                mag->base = (uintptr_t)ptr;
                mag->n = n;
                mag->ndx = 1;
                mag->bkt = bkt;
                for (ndx = 1 ; ndx < n ; ndx++) {
                    u8ptr += bsz;
                    mag->ptab[ndx] = u8ptr;
                }
                mag->prev = NULL;
                mag->next = NULL;
                if (n > 1) {
                    mag->next = magtab[bkt];
                    magtab[bkt] = mag;
                }
            }
        }
    }
    if (ptr) {
#if defined(MEMPARANOIA)
#if ((MEMSLABMINLOG2 - MEMMINLOG2) < (LONGSIZELOG2 + 3))
        bmap = &mag->bmap;
#else
        bmap = mag->bmap;
#endif
        ndx = ((uintptr_t)ptr - mag->base) >> bkt;
        if (bitset(bmap, ndx)) {
            kprintf("duplicate allocation %p (%ld/%ld)\n",
                    ptr, ndx, mag->n);

            panic(k_curproc->pid, TRAPNONE, -EINVAL);
        }
        setbit(bmap, ndx);
#endif /* defined(MEMPARANOIA) */
        if (!slab && (flg & MEMZERO)) {
            kbzero(ptr, 1UL << bkt);
        }
    }
    if (!ptr) {
        panic(k_curproc->pid, TRAPNONE, -ENOMEM);
    }
    mtxunlk(&hdr->lk);

    return ptr;
}

/* TODO: deal with unmapping/freeing physical memory */
void
kfree(void *ptr)
{
    struct mempool *physpool = &memphyspool;
    struct mempool *virtpool = &memvirtpool;
    struct memmag  *mag = memgetmag(ptr, virtpool);
    unsigned long   bkt = (mag) ? mag->bkt : 0;
#if defined(MEMPARANOIA)
    unsigned long   ndx;
    unsigned long  *bmap;
#endif
    struct membkt  *hdr = &virtpool->tab[bkt];
    struct memmag  *list = hdr->list;

    if (!ptr || !mag) {

        return;
    }
    mtxlk(&hdr->lk);
#if defined(MEMPARANOIA)
    ndx = ((uintptr_t)ptr - mag->base) >> bkt;
#if ((MEMSLABMINLOG2 - MEMMINLOG2) < (LONGSIZELOG2 + 3))
    bmap = &mag->bmap;
#else
    bmap = mag->bmap;
#endif
    if (!bitset(bmap, ndx)) {
        kprintf("invalid free: %p (%ld/%ld)\n",
                ptr, ndx, mag->n);

        panic(k_curproc->pid, TRAPNONE, -EINVAL);
    }
#endif
    mempush(mag, ptr);
    if (memmagfull(mag)) {
        if (gtpow2(mag->n, 1)) {
            if ((mag->prev) && (mag->next)) {
                mag->prev->next = mag->next;
                mag->next->prev = mag->prev;
            } else if (mag->prev) {
                mag->prev->next = NULL;
            } else if (mag->next) {
                mag->next->prev = NULL;
                hdr->list = mag->next;
            } else {
                hdr->list = NULL;
            }
        }
        slabfree(physpool, ptr);
        mag->base = 0;
    } else if (mag->ndx == mag->n - 1) {
        mag->prev = NULL;
        if (list) {
            list->prev = mag;
        }
        mag->next = list;
        hdr->list = mag;
    }
#if defined(MEMPARANOIA)
    clrbit(bmap, ndx);
#endif
    mtxunlk(&hdr->lk);

    return;
}

unsigned long
meminitpool(struct mempool *physpool, uintptr_t base, size_t nb)
{
    uintptr_t adr = base;
//    unsigned long sz = (nb & (MEMMIN - 1)) ? rounddownpow2(nb, MEMMIN) : nb;
    size_t    sz = nb;
    intptr_t  ofs = base & (MEMMIN - 1);
    size_t    nblk;
    size_t    hdrsz;

    if (ofs) {
        adr += MEMMIN - ofs;
        sz -= adr - base;
    }
    nblk = sz >> MEMMINLOG2;
    /* configure slab headers */
    hdrsz = nblk * sizeof(struct memslab);
    hdrsz = rounduppow2(hdrsz, PAGESIZE);
#if (__KERNEL__)
    kprintf("MEM: reserved %lu bytes for %lu slab headers\n", hdrsz, nblk);
#endif
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE);
    physpool->nblk = nblk;
    physpool->blktab = (void *)adr;
    adr += hdrsz;
//    kbzero((void *)adr, hdrsz);
    /* configure magazine headers */
    hdrsz = nblk * sizeof(struct memmag);
    hdrsz = rounduppow2(hdrsz, PAGESIZE);
#if (__KERNEL__)
    kprintf("MEM: reserved %lu bytes for %lu magazine headers\n", hdrsz, nblk);
#endif
    memvirtpool.nblk = nblk;
    memvirtpool.blktab = (void *)adr;
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE);
//    kbzero((void *)adr, hdrsz);
    adr += hdrsz;
    memvirtpool.base = adr;
    memphyspool.base = adr;
#if (__KERNEL__ && (MEMDIAG))
    memdiag(memvirtpool);
#endif

    return adr;
}

