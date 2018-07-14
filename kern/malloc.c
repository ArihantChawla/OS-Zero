#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#if (MEMTEST)
#include <stdlib.h>
#endif
#include <errno.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <zero/trix.h>
#include <mt/mtx.h>
#include <kern/util.h>
#include <kern/cpu.h>
#include <kern/mem/vm.h>
#include <kern/mem/mem.h>
#include <kern/mem/slab.h>
#include <kern/proc/proc.h>
#if defined(__i386__)
#include <kern/unit/x86/link.h>
#include <kern/unit/x86/trap.h>
#endif
#if (MEMTEST)
#include <stdio.h>
#include <string.h>
#define kprintf printf
#define kbzero bzero
#define panic(pid, trap, err) abort()
#endif
//#define panic(pid, trap, err)

extern struct memzone memzonetab[MEM_ZONES];

void *
memalloc(size_t nb, long flg)
{
    struct memzone *physzone = &memzonetab[MEM_PHYS_ZONE];
    struct memzone *virtzone = &memzonetab[MEM_VIRT_ZONE];
    void           *ptr = NULL;
    size_t          sz = max(MEMMINSIZE, nb);
    size_t          bsz;
    long            vmflg;
    unsigned long   slab = 0;
    unsigned long   bktid = memcalcbkt(sz);
#if defined(MEMPARANOIA)
    unsigned long  *bmap;
#endif
//    struct memmag   *mag;
    uint8_t        *u8ptr;
    unsigned long   ndx;
    unsigned long   n;
    struct membkt  *bkt = &virtzone->tab[bktid];
    struct memmag  *mag;

    vmflg = PAGEPRES | PAGEWRITE;
    if (MEMWIRE) {
        vmflg |= PAGEPRES;
    }
    fmtxlk(&bkt->lk);
    if (bktid >= MEMSLABSHIFT) {
        ptr = slaballoc(virtzone, sz, flg);
        if (ptr) {
#if (!MEMTEST)
            vminitvirt(ptr, sz, vmflg);
#endif
            slab++;
            mag = memgetmag(ptr, virtzone);
            mag->base = (uintptr_t)ptr;
            mag->n = 1;
            mag->ndx = 1;
            memsetmagbkt(mag, bktid);
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
            ptr = slaballoc(virtzone, sz, flg);
            if (ptr) {
#if (!MEMTEST)
                vminitvirt(ptr, sz, vmflg);
#endif
                u8ptr = ptr;
                slab++;
                bsz = (uintptr_t)1 << bktid;
                n = (uintptr_t)1 << (MEMSLABSHIFT - bktid);
                mag = memgetmag(ptr, virtzone);
                mag->base = (uintptr_t)ptr;
                mag->n = n;
                mag->ndx = 1;
                memsetmagbkt(mag, bktid);
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

            panic(TRAPNONE, -EINVAL);
        }
        setbit(bmap, ndx);
#endif /* defined(MEMPARANOIA) */
        if (!slab && (flg & MEMZERO)) {
            kbzero(ptr, 1UL << bktid);
        }
    }
    if (!ptr) {
        panic(TRAPNONE, -ENOMEM);
    }
    fmtxunlk(&bkt->lk);

    return ptr;
}

void *
memwtalloc(size_t nb, long flg, long spin)
{
    void *ptr = NULL;
    long  nloop;

    do {
        nloop = spin;
        do {
            ptr = memalloc(nb, flg);
            if (ptr) {

                return ptr;
            }
        } while (spin--);
        m_waitint();
    } while (1);

    return ptr;
}

/* TODO: deal with unmapping/freeing physical memory */
void
kfree(void *ptr)
{
    struct memzone *physzone = &memzonetab[MEM_PHYS_ZONE];
    struct memzone *virtzone = &memzonetab[MEM_VIRT_ZONE];
    struct memmag  *mag = memgetmag(ptr, virtzone);
    unsigned long   bktid = (mag) ? memmaggetbkt(mag) : 0;
#if defined(MEMPARANOIA)
    unsigned long   ndx;
    unsigned long  *bmap;
#endif
    struct membkt  *bkt = &virtzone->tab[bktid];
    struct memmag  *head;

    if (!ptr || !mag) {

        return;
    }
    fmtxlk(&bkt->lk);
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

        panic(TRAPNONE, -EINVAL);
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
        slabfree(physzone, ptr);
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
    fmtxunlk(&bkt->lk);

    return;
}

