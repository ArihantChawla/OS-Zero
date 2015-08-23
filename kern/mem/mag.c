#include <stddef.h>
#include <stdint.h>
#if (MEMTEST)
#include <stdlib.h>
#endif
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem/mem.h>
#include <kern/mem/mag.h>
#if defined(__i386__)
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#endif
#if defined(__x86_64__) || defined(__amd64__)
#include <kern/mem/slab64.h>
#else
#include <kern/mem/slab32.h>
#endif

#if (MEMTEST)
#include <stdio.h>
#include <string.h>
#define kprintf printf
#define kbzero bzero
#define kpanic() abort()
#endif

extern struct memzone slabvirtzone;
struct memzone        magvirtzone;

void *
memalloc(unsigned long nb, long flg)
{
    struct memzone  *slabzone = &slabvirtzone;
    struct memzone  *magzone = &magvirtzone;
    volatile long   *lktab = magzone->lktab;
    struct maghdr  **magtab = (struct maghdr **)(magzone->tab);
    void            *ptr = NULL;
    unsigned long    sz = max(MAGMIN, nb);
    unsigned long    slab = 0;
    unsigned long    bkt = memgetbkt(sz);
    struct maghdr   *mag;
    void            *bmap;
    uint8_t         *u8ptr;
    unsigned long    l;
    unsigned long    n;
    unsigned long    ndx;
    unsigned long    mlk = 0;

    mtxlk(&lktab[bkt]);
    if (bkt >= SLABMINLOG2) {
#if (MEMTEST)
        ptr = slaballoc(slabzone, sz, flg);
#else
        ptr = vmmapvirt((uint32_t *)&_pagetab,
                        slaballoc(slabzone, sz, flg),
                        sz, flg);
#endif
        if (ptr) {
            slab++;
            mag = maggethdr(ptr, magzone);
            mtxlk(&mag->lk);
            mlk++;
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
            mtxlk(&mag->lk);
            mlk++;
            ptr = magpop(mag);
            if (magempty(mag)) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                magtab[bkt] = mag->next;
            }
        } else {
#if (MEMTEST)
            ptr = u8ptr = slaballoc(slabzone, SLABMIN, flg);
#else
            ptr = u8ptr = vmmapvirt((uint32_t *)&_pagetab,
                                    slaballoc(slabzone, SLABMIN, flg),
                                    SLABMIN, flg);
#endif
            if (ptr) {
                slab++;
                sz = 1UL << bkt;
                n = 1UL << (SLABMINLOG2 - bkt);
                mag = maggethdr(ptr, magzone);
                mtxlk(&mag->lk);
                mlk++;
                mag->base = (uintptr_t)ptr;
                mag->n = n;
                mag->ndx = 1;
                mag->bkt = bkt;
                for (l = 1 ; l < n ; l++) {
                    u8ptr += sz;
                    mag->ptab[l] = u8ptr;
                }
                mag->prev = NULL;
                if (magtab[bkt]) {
                    magtab[bkt]->prev = mag;
                }
                mag->next = magtab[bkt];
                magtab[bkt] = mag;
            }
        }
    }
    if (ptr) {
#if ((SLABMINLOG2 - MAGMINLOG2) < (LONGSIZELOG2 + 3))
        bmap = &mag->bmap;
#else
        bmap = mag->bmap;
#endif
        ndx = ((uintptr_t)ptr - mag->base) >> bkt;
        if (bitset(bmap, ndx)) {
            kprintf("duplicate allocation %p (%ld/%ld)\n",
                    ptr, ndx, mag->n);

            kpanic();
        }
        setbit(bmap, ndx);
        if (!slab && (flg & MEMZERO)) {
            kbzero(ptr, 1UL << bkt);
        }
    }
    if (!ptr) {
        kpanic();
    }
    if (mlk) {
        mtxunlk(&mag->lk);
    }
    mtxunlk(&lktab[bkt]);

    return ptr;
}

/* TODO: deal with unmapping/freeing physical memory */
void
kfree(void *ptr)
{
    struct memzone  *slabzone = &slabvirtzone;
    struct memzone  *magzone = &magvirtzone;
    volatile long   *lktab = magzone->lktab;
    struct maghdr  **magtab = (struct maghdr **)(magzone->tab);
    struct maghdr   *mag = maggethdr(ptr, magzone);
#if ((SLABMINLOG2 - MAGMINLOG2) < (LONGSIZELOG2 + 3))
    void            *bmap = &mag->bmap;
#else
    void            *bmap = mag->bmap;
#endif
    unsigned long    bkt = (mag) ? mag->bkt : 0;
    unsigned long    ndx;
    unsigned long    freed = 0;

    if (!ptr) {

        return;
    }
    mtxlk(&lktab[bkt]);
    mtxlk(&mag->lk);
    ndx = ((uintptr_t)ptr - mag->base) >> bkt;
    if (!bitset(bmap, ndx)) {
        kprintf("invalid free: %p (%ld/%ld)\n",
                ptr, ndx, mag->n);

        kpanic();
    }
    magpush(mag, ptr);
    if (magfull(mag)) {
        if (gtpow2(mag->n, 1)) {
            if ((mag->prev) && (mag->next)) {
                mag->prev->next = mag->next;
                mag->next->prev = mag->prev;
            } else if (mag->prev) {
                mag->prev->next = NULL;
            } else if (mag->next) {
                mag->next->prev = NULL;
                magtab[bkt] = mag->next;
            } else {
                magtab[bkt] = NULL;
            }
        }
        slabfree(slabzone, ptr);
        freed = 1;
    } else if (mag->ndx == mag->n) {
        mag->prev = NULL;
        if (magtab[bkt]) {
            magtab[bkt]->prev = mag;
        }
        mag->next = magtab[bkt];
        magtab[bkt] = mag;
    }
    clrbit(bmap, ndx);
    if (freed) {
        mag->base = 0;
    }
    mtxunlk(&mag->lk);
    mtxunlk(&lktab[bkt]);

    return;
}

