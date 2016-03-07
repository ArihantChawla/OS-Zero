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
    void            *ptr = NULL;
    size_t           sz = max(MEMMINSIZE, nb);
    size_t           bsz;
    unsigned long    slab = 0;
    unsigned long    bktid = memcalcbkt(sz);
#if defined(MEMPARANOIA)
    unsigned long   *bmap;
#endif
//    struct memmag   *mag;
    uint8_t         *u8ptr;
    unsigned long    ndx;
    unsigned long    n;
    struct membkt   *bkt = &virtpool->tab[bktid];
    struct memmag   *mag;

    mtxlk(&bkt->lk);
    if (bktid >= MEMSLABSHIFT) {
        ptr = slaballoc(virtpool, sz, flg);
        if (ptr) {
#if (!MEMTEST)
            vminitvirt(&_pagetab, ptr, sz, flg);
#endif
            slab++;
            mag = memgetmag(ptr, virtpool);
            mag->base = (uintptr_t)ptr;
            mag->n = 1;
            mag->ndx = 1;
            memmagsetbkt(mag, bktid);
            mag->prev = NULL;
            mag->next = NULL;
        }
    } else {
        mag = bkt->list;
        if (mag) {
            ptr = mempop(mag);
            if (memmagempty(mag)) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                bkt->list = mag->next;
            }
        } else {
            ptr = slaballoc(virtpool, sz, flg);
            if (ptr) {
#if (!MEMTEST)
                vminitvirt(&_pagetab, ptr, sz, flg);
#endif
                u8ptr = ptr;
                slab++;
                bsz = (uintptr_t)1 << bktid;
                n = (uintptr_t)1 << (MEMSLABSHIFT - bktid);
                mag = memgetmag(ptr, virtpool);
                mag->base = (uintptr_t)ptr;
                mag->n = n;
                mag->ndx = 1;
                memmagsetbkt(mag, bktid);
                for (ndx = 1 ; ndx < n ; ndx++) {
                    u8ptr += bsz;
                    mag->ptab[ndx] = u8ptr;
                }
                mag->prev = NULL;
                mag->next = NULL;
                if (n > 1) {
                    mag->next = bkt->list;
                    bkt->list = mag;
                }
            }
        }
    }
    if (ptr) {
#if defined(MEMPARANOIA)
#if ((MEMSLABSHIFT - MEMMINSHIFT) < (LONGSIZELOG2 + 3))
        bmap = &mag->bmap;
#else
        bmap = mag->bmap;
#endif
        ndx = ((uintptr_t)ptr - mag->base) >> bktid;
        if (bitset(bmap, ndx)) {
            kprintf("duplicate allocation %p (%ld/%ld)\n",
                    ptr, ndx, mag->n);

            panic(k_curproc->pid, TRAPNONE, -EINVAL);
        }
        setbit(bmap, ndx);
#endif /* defined(MEMPARANOIA) */
        if (!slab && (flg & MEMZEROBIT)) {
            kbzero(ptr, 1UL << bktid);
        }
    }
    if (!ptr) {
        panic(k_curproc->pid, TRAPNONE, -ENOMEM);
    }
    mtxunlk(&bkt->lk);

    return ptr;
}

/* TODO: deal with unmapping/freeing physical memory */
void
kfree(void *ptr)
{
    struct mempool *physpool = &memphyspool;
    struct mempool *virtpool = &memvirtpool;
    struct memmag  *mag = memgetmag(ptr, virtpool);
    unsigned long   bktid = (mag) ? memmaggetbkt(mag) : 0;
#if defined(MEMPARANOIA)
    unsigned long   ndx;
    unsigned long  *bmap;
#endif
    struct membkt  *bkt = &virtpool->tab[bktid];
    struct memmag  *head;

    if (!ptr || !mag) {

        return;
    }
    mtxlk(&bkt->lk);
#if defined(MEMPARANOIA)
    ndx = ((uintptr_t)ptr - mag->base) >> bktid;
#if ((MEMSLABSHIFT - MEMMINSHIFT) < (LONGSIZELOG2 + 3))
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
                bkt->list = mag->next;
            } else {
                bkt->list = NULL;
            }
        }
        slabfree(physpool, ptr);
        mag->base = 0;
    } else if (mag->ndx == mag->n - 1) {
        head = bkt->list;
        mag->prev = NULL;
        if (head) {
            head->prev = mag;
        }
        mag->next = head;
        bkt->list = mag;
    }
#if defined(MEMPARANOIA)
    clrbit(bmap, ndx);
#endif
    mtxunlk(&bkt->lk);

    return;
}

unsigned long
meminitpool(struct mempool *pool, uintptr_t base, size_t nbyte)
{
    uintptr_t adr = base;
//    unsigned long sz = (nbyte & (MEMSLABSIZE - 1)) ? rounddownpow2(nbyte, MEMSLABSIZE) : nbyte;
    size_t    sz = nbyte;
    intptr_t  ofs = base & (MEMSLABSIZE - 1);
    size_t    nblk;
    size_t    hdrsz;

    if (ofs) {
        adr += MEMSLABSIZE - ofs;
        sz -= adr - base;
    }
    nblk = sz >> MEMSLABSHIFT;
#if 0
    /* configure slab headers */
    hdrsz = nblk * sizeof(struct memslab);
    hdrsz = rounduppow2(hdrsz, MEMSLABSIZE);
#if (__KERNEL__)
    kprintf("MEM: reserved %lu bytes for %lu slab headers\n", hdrsz, nblk);
#endif
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE | PAGEWIRED);
    pool->nblk = nblk;
    pool->hdrtab = (void *)adr;
    adr += hdrsz;
    kbzero((void *)adr, hdrsz);
#endif
    /* configure magazine headers */
    hdrsz = nblk * sizeof(struct memmag);
    hdrsz = rounduppow2(hdrsz, MEMSLABSIZE);
#if (__KERNEL__)
    kprintf("MEM: reserved %lu bytes for %lu magazine headers\n", hdrsz, nblk);
#endif
    memvirtpool.nblk = nblk;
    memvirtpool.hdrtab = (void *)adr;
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE | PAGEWIRED);
    kbzero((void *)adr, hdrsz);
    adr += hdrsz;
    pool->base = adr;
#if (__KERNEL__ && (MEMDIAG))
    memdiag(memvirtpool);
#endif

    return adr;
}

