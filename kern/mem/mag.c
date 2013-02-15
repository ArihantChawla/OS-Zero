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
volatile long *magvirtbitmap;
#endif
#if (MAGSLABLK)
volatile long  magslablk;
#endif

#if (SMP)
static __inline__ long
magbitset(volatile long *map, unsigned long ndx)
{
    m_membar();

    return (bitset(map, ndx));
}

static __inline__ void
magsetbit(volatile long *map, unsigned long ndx)
{
    m_membar();
    setbit(map, ndx);
}

static __inline__ void
magclrbit(volatile long *map, unsigned long ndx)
{
    m_membar();
    clrbit(map, ndx);
}
#else
#define magbitset(map, ndx) bitset(map, ndx)
#define magsetbit(map, ndx) setbit(map, ndx)
#define magclrbit(map, ndx) clrbit(map, ndx)
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
    maglkq(magvirtlktab, bkt);
    if (bkt >= SLABMINLOG2) {
#if (MAGSLABLK)
        mtxlk(&magslablk);
#endif
#if (MEMTEST)
        ptr = slaballoc(slabvirttab, slabvirthdrtab, sz, flg);
#else
        ptr = vmmapvirt((uint32_t *)&_pagetab,
                        slaballoc(slabvirttab, slabvirthdrtab, sz, flg),
                        sz, flg);
#endif
#if (MAGSLABLK)
        mtxunlk(&magslablk);
#endif
        if (ptr) {
            slab++;
            mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
#if (MAGLK)
            mtxlk(&mag->lk);
            mlk++;
#endif
#if (MAGBITMAP)
            mag->base = (uintptr_t)ptr;
#endif
            mag->n = 1;
            mag->ndx = 1;
            mag->bkt = bkt;
            mag->prev = NULL;
            mag->next = NULL;
#if (MAGBITMAP)
//            kbzero(mag->bmap, sizeof(mag->bmap));
#endif
        }
    } else {
        mag = magvirttab[bkt];
        if (mag) {
#if (MAGLK)
            mtxlk(&mag->lk);
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
            mtxlk(&magslablk);
#endif
            ptr = u8ptr = slaballoc(slabvirttab, slabvirthdrtab, SLABMIN, flg);
#if (MAGSLABLK)
            mtxunlk(&magslablk);
#endif
            if (ptr) {
                sz = 1UL << bkt;
                n = 1UL << (SLABMINLOG2 - bkt);
                mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
#if (MAGLK)
                mtxlk(&mag->lk);
                mlk++;
#endif
#if (MAGBITMAP)
                mag->base = (uintptr_t)ptr;
#endif
                mag->n = n;
                mag->bkt = bkt;
#if (MAGBITMAP)
//                kbzero(mag->bmap, sizeof(mag->bmap));
#endif
#if (MEMTEST)
#endif
                for (l = 1 ; l < n ; l++) {
                    u8ptr += sz;
                    mag->ptab[l] = u8ptr;
                }
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
    }
    if (ptr) {
#if (MAGBITMAP)
        ndx = ((uintptr_t)ptr - mag->base) >> bkt;
        if (magbitset(mag->bmap, ndx)) {
            kprintf("duplicate allocation %p (%ld/%ld)\n",
                    ptr, ndx, mag->n);

            kpanic();
        }
        magsetbit(mag->bmap, ndx);
#else
        ndx = ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2;
        if (magbitset(magvirtbitmap, ndx)) {
            kprintf("duplicate allocation %p\n", ptr);
            
            kpanic();
        }
        magsetbit(magvirtbitmap, ndx);
#endif
        if (!slab && (flg & MEMZERO)) {
            kbzero(ptr, 1UL << bkt);
        }
    }
#if (MAGLK)
    if (mlk) {
        mtxunlk(&mag->lk);
    }
#endif
    magunlkq(magvirtlktab, bkt);

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
    maglkq(magvirtlktab, bkt);
#if (MAGLK)
    mtxlk(&mag->lk);
#endif
#if (MAGBITMAP)
    ndx = ((uintptr_t)ptr - mag->base) >> bkt;
    if (!magbitset(mag->bmap, ndx)) {
        kprintf("invalid free: %p (%ld/%ld)\n",
                ptr, ndx, mag->n);

        kpanic();
    }
#else
    ndx = ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2;
    if (!magbitset(magvirtbitmap, ndx)) {
        kprintf("invalid free: %p\n", ptr);

        kpanic();
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
                magvirttab[bkt] = mag->next;
            } else {
                magvirttab[bkt] = NULL;
            }
        }
#if (MAGSLABLK)
        mtxlk(&magslablk);
#endif
        slabfree(slabvirttab, slabvirthdrtab, ptr);
#if (MAGSLABLK)
        mtxunlk(&magslablk);
#endif
#if (MAGBITMAP)
        freed = 1;
#endif
    } else if (mag->ndx == mag->n) {
        mag->prev = NULL;
        if (magvirttab[bkt]) {
            magvirttab[bkt]->prev = mag;
        }
        mag->next = magvirttab[bkt];
        magvirttab[bkt] = mag;
    }
#if (MAGBITMAP)
    magclrbit(mag->bmap, ndx);
    if (freed) {
        mag->base = 0;
    }
#else
    magclrbit(magvirtbitmap, ndx);
#endif
#if (MAGLK)
    mtxunlk(&mag->lk);
#endif
    magunlkq(magvirtlktab, bkt);

    return;
}

