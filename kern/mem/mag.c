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

extern unsigned long   slabvirtbase;
extern struct slabhdr *slabvirthdrtab;
extern struct slabhdr *slabvirttab[];

struct maghdr *magvirttab[PTRBITS] ALIGNED(PAGESIZE);
volatile long  magvirtlktab[PTRBITS];
struct maghdr *magvirthdrtab;
#if (!MAGBITMAP)
uint8_t       *magvirtbitmap;
#endif
#if (MAGSLABLK)
volatile long  magslablk;
#endif

void *
memalloc(unsigned long nb, long flg)
{
    void          *ptr = NULL;
    unsigned long  sz = max(MAGMIN, nb);
    unsigned long  slab = 0;
    unsigned long  bkt;
    struct maghdr *mag;
    uint8_t       *u8ptr;
    unsigned long  l;
    unsigned long  n;
    unsigned long  ndx;
#if (MAGLK)
    unsigned long  mlk = 0;
#endif

    bkt = memgetbkt(sz);
#if (MAGNEWLK)
    maglkq(magvirtlktab, bkt);
#endif
    if (bkt >= SLABMINLOG2) {
#if (MAGSLABLK)
        mtxlk(&magslablk, MEMPID);
#endif
#if (MEMTEST)
        ptr = slaballoc(slabvirttab, slabvirthdrtab, sz, flg);
#else
        ptr = vmmapvirt((uint32_t *)&_pagetab,
                        slaballoc(slabvirttab, slabvirthdrtab, sz, flg),
                        sz, flg);
#endif
#if (MAGSLABLK)
        mtxunlk(&magslablk, MEMPID);
#endif
        if (ptr) {
            slab++;
            mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
#if (MAGLK)
            mtxlk(&mag->lk, MEMPID);
            mlk++;
#endif
#if (MAGBITMAP)
            mag->base = (uintptr_t)ptr;
#endif
            mag->n = 1;
            mag->ndx = 1;
#if (KLUDGES)
            mag->bkt = bkt;
            mag->prev = NULL;
            mag->next = NULL;
#endif
#if (MAGBITMAP)
            clrbit(mag->bmap, 0);
#endif
        }
    } else {
#if (!MAGNEWLK)
        maglkq(magvirtlktab, bkt);
#endif
        mag = magvirttab[bkt];
        if (mag) {
#if (MAGLK)
            mtxlk(&mag->lk, MEMPID);
            mlk++;
#endif
            ptr = magpop(mag);
            if (magempty(mag)) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                magvirttab[bkt] = mag->next;
            }
        } else {
#if (MAGSLABLK)
            mtxlk(&magslablk, MEMPID);
#endif
            ptr = u8ptr = slaballoc(slabvirttab, slabvirthdrtab, SLABMIN, flg);
#if (MAGSLABLK)
            mtxunlk(&magslablk, MEMPID);
#endif
            if (ptr) {
                sz = 1UL << bkt;
                n = 1UL << (SLABMINLOG2 - bkt);
                mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
#if (MAGLK)
                mtxlk(&mag->lk, MEMPID);
                mlk++;
#endif
#if (MAGBITMAP)
                mag->base = (uintptr_t)ptr;
#endif
                mag->n = n;
                mag->bkt = bkt;
#if (MAGBITMAP)
                kbzero(mag->bmap, sizeof(mag->bmap));
#endif
#if (MEMTEST)
#endif
#if (KLUDGES)
                for (l = 1 ; l < n ; l++) {
                    u8ptr += sz;
                    mag->ptab[l] = u8ptr;
                }
#else
                for (l = 0 ; l < n ; l++) {
                    mag->ptab[l] = u8ptr;
                    u8ptr += sz;
                }
#endif
#if (MEMTEST)
#endif
                mag->ndx = 1;
                mag->prev = NULL;
                if (magvirttab[bkt]) {
                    magvirttab[bkt]->prev = mag;
                }
                mag->next = magvirttab[bkt];
                magvirttab[bkt] = mag;
            }
        }
#if (!MAGNEWLK)
        magunlkq(magvirtlktab, bkt);
#endif
    }
    if (ptr) {
#if (MAGBITMAP)
        ndx = ((uintptr_t)ptr - mag->base) >> bkt;
        if (bitset(mag->bmap, ndx)) {
            kprintf("duplicate allocation %p (%ld/%ld)\n",
                    ptr, ndx, mag->n);
            magdiag();
            magprint(mag);
            fflush(stdout);

            kpanic();
        }
        setbit(mag->bmap, ndx);
#else
        ndx = ((uintptr_t)ptr - mag->virtbase) >> MAGMINLOG2;
        if (bitset(magvirtbitmap, ndx)) {
            kprintf("duplicate allocation %p\n", ptr);
            
            kpanic();
        }
        setbit(magvirtbitmap, ndx);
#endif
        if (!slab && (flg & MEMZERO)) {
            kbzero(ptr, 1UL << bkt);
        }
    }
#if (MAGLK)
    if (mlk) {
        mtxunlk(&mag->lk, MEMPID);
    }
#endif
#if (MAGNEWLK)
    magunlkq(magvirtlktab, bkt);
#endif

    return ptr;
}

void
kfree(void *ptr)
{
    struct maghdr *mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
    unsigned long  bkt = (mag) ? mag->bkt : 0;
    unsigned long  ndx;
#if (MAGBITMAP)
    unsigned long  freed = 0;
#endif

    if (!ptr) {

        return;
    }
#if (MAGNEWLK)
    maglkq(magvirtlktab, bkt);
#endif
#if (MAGLK)
    mtxlk(&mag->lk, MEMPID);
#endif
#if (MAGBITMAP)
    ndx = ((uintptr_t)ptr - mag->base) >> bkt;
    if (!bitset(mag->bmap, ndx)) {
        kprintf("invalid free: %p (%ld/%ld)\n",
                ptr, ndx, mag->n);
        magprint(mag);

        kpanic();
    }
#else
    ndx = ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2;
    if (!bitset(magvirtbitmap, ndx)) {
        kprintf("invalid free: %p\n", ptr);

        kpanic();
    }
#endif
    magpush(mag, ptr);
    if (magfull(mag)) {
        if (mag->n > 1) {
#if (!MAGNEWLK)
            maglkq(magvirtlktab, bkt);
#endif
            if ((mag->prev) && (mag->next)) {
                mag->prev->next = mag->next;
                mag->next->prev = mag->prev;
            } else if (mag->prev) {
                mag->prev->next = NULL;
            } else if (mag->next) {
                mag->next->prev = NULL;
                magvirttab[bkt] = mag->next;
            } else {
                magvirttab[bkt] = NULL;
            }
#if (!MAGNEWLK)
            magunlkq(magvirtlktab, bkt);
#endif
        }
#if (MAGSLABLK)
        mtxlk(&magslablk, MEMPID);
#endif
        slabfree(slabvirttab, slabvirthdrtab, ptr);
#if (MAGSLABLK)
        mtxunlk(&magslablk, MEMPID);
#endif
#if (KLUDGES)
        freed = 1;
#endif
    } else if (mag->ndx == mag->n) {
        mag->prev = NULL;
#if (!MAGNEWLK)
        maglkq(magvirtlktab, bkt);
#endif
        if (magvirttab[bkt]) {
            magvirttab[bkt]->prev = mag;
        }
        mag->next = magvirttab[bkt];
        magvirttab[bkt] = mag;
#if (!MAGNEWLK)
        magunlkq(magvirtlktab, bkt);
#endif
    }
#if (MAGBITMAP)
    clrbit(mag->bmap, ndx);
    if (freed) {
        mag->base = 0;
    }
#else
    clrbit(magvirtbitmap, ndx);
#endif
#if (MAGLK)
    mtxunlk(&mag->lk, MEMPID);
#endif
#if (MAGNEWLK)
    magunlkq(magvirtlktab, bkt);
#endif

    return;
}

