#include <stddef.h>
#include <stdint.h>
#if (MEMTEST)
#include <stdlib.h>
#endif
#include <errno.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem/vm.h>
#include <kern/mem/mem.h>
#include <kern/mem/mag.h>
#if defined(__i386__)
#include <kern/unit/x86/link.h>
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
#define panic(x) abort()
#endif

extern struct memzone slabvirtzone;
struct memzone        magvirtzone;

void *
memalloc(unsigned long nb, long flg)
{
    struct memzone  *slabzone = &slabvirtzone;
    struct memzone  *magzone = &magvirtzone;
#if (!MEM_AVOID_CACHELINE_SHARE)
    volatile long   *lktab = magzone->lktab;
    struct maghdr  **magtab = (struct maghdr **)magzone->tab;
#endif
    void            *ptr = NULL;
    unsigned long    sz = max(MAGMIN, nb);
    unsigned long    slab = 0;
    unsigned long    bkt = memgetbkt(sz);
    struct maghdr   *mag;
//    void            *bmap;
    uint8_t         *u8ptr;
    unsigned long    ndx;
    unsigned long    n;
#if (MEM_AVOID_CACHELINE_SHARE)
    struct memhdr   *hdr = &magzone->tab[bkt];
#endif
//    unsigned long    mlk = 0;

#if (MEM_AVOID_CACHELINE_SHARE)
    mtxlk(&hdr->lk);
#else
    mtxlk(&lktab[bkt]);
#endif
    if (bkt >= SLABMINLOG2) {
        ptr = slaballoc(slabzone, sz, flg);
        if (ptr) {
#if (!MEMTEST)
            vminitvirt(&_pagetab, ptr, sz, flg);
#endif
            slab++;
            mag = maggethdr(ptr, magzone);
//            mtxlk(&mag->lk);
//            mlk++;
#if 0
#if ((SLABMINLOG2 - MAGMINLOG2) < (LONGSIZELOG2 + 3))
            mag->bmap = 0L;
            bmap = &mag->bmap;
#else
            bmap = mag->bmap;
            kbzero(bmap, 1UL << (SLABMINLOG2 - MAGMINLOG2 - 3));
#endif
#endif
            mag->base = (uintptr_t)ptr;
            mag->n = 1;
            mag->ndx = 1;
            mag->bkt = bkt;
            mag->prev = NULL;
            mag->next = NULL;
        }
    } else {
#if (MEM_AVOID_CACHELINE_SHARE)
        mag = hdr->list;
#else
        mag = magtab[bkt];
#endif
        if (mag) {
//            mtxlk(&mag->lk);
//            mlk++;
            ptr = magpop(mag);
            if (magempty(mag)) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
#if (MEM_AVOID_CACHELINE_SHARE)
                hdr->list = mag->next;
#else
                magtab[bkt] = mag->next;
#endif
            }
        } else {
//            ptr = slaballoc(slabzone, SLABMIN, flg);
            ptr = slaballoc(slabzone, sz, flg);
            if (ptr) {
#if (!MEMTEST)
                vminitvirt(&_pagetab, ptr, SLABMIN, flg);
#endif
#if 0
#if ((SLABMINLOG2 - MAGMINLOG2) < (LONGSIZELOG2 + 3))
                mag->bmap = 0;
                bmap = &mag->bmap;
#else
                bmap = mag->bmap;
                kbzero(bmap, 1UL << (SLABMINLOG2 - MAGMINLOG2 - 3));
#endif
#endif
                u8ptr = ptr;
                slab++;
                sz = 1UL << bkt;
                n = 1UL << (SLABMINLOG2 - bkt);
                mag = maggethdr(ptr, magzone);
//                mtxlk(&mag->lk);
//                mlk++;
                mag->base = (uintptr_t)ptr;
                mag->n = n;
                mag->ndx = 1;
                mag->bkt = bkt;
                for (ndx = 1 ; ndx < n ; ndx++) {
                    u8ptr += sz;
                    mag->ptab[ndx] = u8ptr;
                }
                mag->prev = NULL;
#if (MEM_AVOID_CACHELINE_SHARE)
                hdr->list = mag;
#else
#if 0
                if (magtab[bkt]) {
                    magtab[bkt]->prev = mag;
                }
#endif
                mag->next = magtab[bkt];
                magtab[bkt] = mag;
#endif
            }
        }
    }
    if (ptr) {
#if 0
#if ((SLABMINLOG2 - MAGMINLOG2) < (LONGSIZELOG2 + 3))
        bmap = &mag->bmap;
#else
        bmap = mag->bmap;
#endif
        ndx = ((uintptr_t)ptr - mag->base) >> bkt;
        if (bitset(bmap, ndx)) {
            kprintf("duplicate allocation %p (%ld/%ld)\n",
                    ptr, ndx, mag->n);

            panic(-EINVAL);
        }
        setbit(bmap, ndx);
#endif
        if (!slab && (flg & MEMZERO)) {
            kbzero(ptr, 1UL << bkt);
        }
    }
    if (!ptr) {
        panic(-ENOMEM);
    }
#if 0
    if (mlk) {
//        mtxunlk(&mag->lk);
    }
#endif
#if (MEM_AVOID_CACHELINE_SHARE)
    mtxunlk(&hdr->lk);
#else
    mtxunlk(&lktab[bkt]);
#endif

    return ptr;
}

/* TODO: deal with unmapping/freeing physical memory */
void
kfree(void *ptr)
{
    struct memzone  *slabzone = &slabvirtzone;
    struct memzone  *magzone = &magvirtzone;
#if (!MEM_AVOID_CACHELINE_SHARE)
    volatile long   *lktab = magzone->lktab;
    struct maghdr  **magtab = (struct maghdr **)magzone->tab;
#endif
    struct maghdr   *mag = maggethdr(ptr, magzone);
#if 0
#if ((SLABMINLOG2 - MAGMINLOG2) < (LONGSIZELOG2 + 3))
    void            *bmap = (mag) ? &mag->bmap : NULL;
#else
    void            *bmap = (mag) ? mag->bmap : NULL;
#endif
#endif
    unsigned long    bkt = (mag) ? mag->bkt : 0;
    unsigned long    ndx;
    unsigned long    freed = 0;
#if (MEM_AVOID_CACHELINE_SHARE)
    struct memhdr   *hdr = &magzone->tab[bkt];
    struct maghdr   *list = hdr->list;
#endif

    if (!ptr || !mag) {

        return;
    }
#if (MEM_AVOID_CACHELINE_SHARE)
    mtxlk(&hdr->lk);
#else
    mtxlk(&lktab[bkt]);
#endif
//    mtxlk(&mag->lk);
    ndx = ((uintptr_t)ptr - mag->base) >> bkt;
#if 0
    if (!bitset(bmap, ndx)) {
        kprintf("invalid free: %p (%ld/%ld)\n",
                ptr, ndx, mag->n);

        panic(-EINVAL);
    }
#endif
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
#if (MEM_AVOID_CACHELINE_SHARE)
                hdr->list = mag->next;
#else
                magtab[bkt] = mag->next;
#endif
            } else {
#if (MEM_AVOID_CACHELINE_SHARE)
                hdr->list = NULL;
#else
                magtab[bkt] = NULL;
#endif
            }
        }
        slabfree(slabzone, ptr);
        freed = 1;
    } else if (mag->ndx == mag->n - 1) {
        mag->prev = NULL;
#if (MEM_AVOID_CACHELINE_SHARE)
        if (list) {
            list->prev = mag;
        }
        mag->next = list;
        hdr->list = mag;
#else
        if (magtab[bkt]) {
            magtab[bkt]->prev = mag;
        }
        mag->next = magtab[bkt];
        magtab[bkt] = mag;
#endif
    }
#if 0
    clrbit(bmap, ndx);
#endif
    if (freed) {
        mag->base = 0;
    }
//    mtxunlk(&mag->lk);
#if (MEM_AVOID_CACHELINE_SHARE)
    mtxunlk(&hdr->lk);
#else
    mtxunlk(&lktab[bkt]);
#endif

    return;
}

