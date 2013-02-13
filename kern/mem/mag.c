#include <stddef.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#if defined(__i386__)
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>
#endif
#include <kern/mem/mem.h>
#include <kern/mem/mag.h>
#if defined(__x86_64__) || defined(__amd64__)
#include <kern/mem/slab64.h>
#else
#include <kern/mem/slab32.h>
#endif

#if (MEMTEST)
#include <stdio.h>
#define kprintf printf
#define panic() abort()
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

void *
memalloc(unsigned long nb, long flg)
{
    void          *ptr = NULL;
    unsigned long  sz = 0;
    unsigned long  slab = 0;
    unsigned long  bkt;
    struct maghdr *mag;
    uint8_t       *u8ptr;
    unsigned long  l;
    unsigned long  n;
#if (MAGLK)
    unsigned long  mlk = 0;
#endif

    nb = max(MAGMIN, nb);
    bkt = memgetbkt(nb);
    if (nb > (SLABMIN >> 1)) {
#if (MEMTEST)
        ptr = slaballoc(slabvirttab, slabvirthdrtab, nb, flg);
#else
        ptr = vmmapvirt((uint32_t *)&_pagetab,
                        slaballoc(slabvirttab, slabvirthdrtab, nb, flg),
                        nb, flg);
#endif
#if (MEMTEST)
//        printf("PTR: %p, MAG == %lx\n", ptr, magnum(ptr, slabvirtbase));
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
            mag->n = 0;
            mag->ndx = 0;
#if (KLUDGES)
            mag->bkt = 0;
            mag->prev = NULL;
            mag->next = NULL;
#endif
#if (MAGBITMAP)
            clrbit(mag->bmap, 0);
#endif
        }
    } else {
        maglkq(magvirtlktab, bkt);
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
            sz = 1UL << bkt;
            ptr = u8ptr = slaballoc(slabvirttab, slabvirthdrtab, SLABMIN, flg);
            if (ptr) {
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
#if (KLUDGES) && 0
                mag->prev = NULL;
                mag->next = NULL;
#endif
#if (MAGBITMAP)
                bzero(mag->bmap, sizeof(mag->bmap));
#endif
#if (MEMTEST)
//            printf("INIT: %ld items:", n);
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
//            printf("\n");
#endif
//            ptr = magpop(mag);
                mag->ndx = 1;
                mag->prev = NULL;
                if (magvirttab[bkt]) {
                    magvirttab[bkt]->prev = mag;
                }
                mag->next = magvirttab[bkt];
                magvirttab[bkt] = mag;
            }
        }
        magunlkq(magvirtlktab, bkt);
    }
#if (MEMTEST)
    magdiag();
#endif
    if (ptr) {
#if (MAGBITMAP)
        if (bitset(mag->bmap, ((uintptr_t)ptr - mag->base) >> bkt)) {
            kprintf("duplicate allocation %p (%ld/%ld)\n",
                    ptr, ((uintptr_t)ptr - mag->base) >> bkt, mag->n);
            magprint(mag);

            panic();
        }
        setbit(mag->bmap, ((uintptr_t)ptr - mag->base) >> bkt);
#else
        if (bitset(magvirtbitmap,
                   ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2)) {
            kprintf("duplicate allocation %p\n", ptr);
            
            panic();
        }
        setbit(magvirtbitmap, ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2);
#endif
        if (!slab && (flg & MEMZERO)) {
            bzero(ptr, 1UL << bkt);
        }
    }
#if (MEMTEST)
//    printf("MAGPTR: %p\n", ptr);
#endif
#if (MAGLK)
    if (mlk) {
        mtxunlk(&mag->lk, MEMPID);
    }
#endif

    return ptr;
}

void
kfree(void *ptr)
{
    struct maghdr *mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
    unsigned long  bkt = (mag) ? mag->bkt : 0;
#if (MAGBITMAP)
    unsigned long  freed = 0;
#endif

    if (!ptr) {

        return;
    }
#if (MAGLK)
    mtxlk(&mag->lk, MEMPID);
#endif
#if (MAGBITMAP)
    if (!bitset(mag->bmap, ((uintptr_t)ptr - mag->base) >> bkt)) {
        kprintf("invalid free: %p (%ld/%ld)\n",
                ptr, ((uintptr_t)ptr - mag->base) >> bkt, mag->n);

        panic();
    }
#else
    if (!bitset(magvirtbitmap, ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2)) {
        kprintf("invalid free: %p\n", ptr);

        panic();
    }
#endif
//    clrbit(magvirtbitmap, ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2);
    if (!mag->n && !mag->ndx) {
#if (MAGBITMAP)
        freed = 0;
#endif
        slabfree(slabvirttab, slabvirthdrtab, ptr);
        mag->n = 0;
        mag->ndx = 0;
    } else {
        magpush(mag, ptr);
        if (magfull(mag)) {
            slabfree(slabvirttab, slabvirthdrtab, ptr);
#if (!MAGBITMAP)
            bkt = mag->bkt;
#endif
            maglkq(magvirtlktab, bkt);
            if (mag->prev) {
                mag->prev->next = mag->next;
            } else {
                magvirttab[bkt] = mag->next;
            }
            if (mag->next) {
                mag->next->prev = mag->prev;
            }
            magunlkq(magvirtlktab, bkt);
        } else if (mag->ndx == mag->n - 1) {
#if (!MAGBITMAP)
            bkt = mag->bkt;
#endif
            mag->prev = NULL;
            maglkq(magvirtlktab, bkt);
            if (magvirttab[bkt]) {
                magvirttab[bkt]->prev = mag;
            }
            mag->next = magvirttab[bkt];
            magvirttab[bkt] = mag;
            magunlkq(magvirtlktab, bkt);
        }
    }
#if (MAGBITMAP)
    clrbit(mag->bmap, ((uintptr_t)ptr - mag->base) >> bkt);
    if (freed) {
        mag->base = 0;
    }
#else
    clrbit(magvirtbitmap, ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2);
#endif
#if (MAGLK)
    mtxunlk(&mag->lk, MEMPID);
#endif
#if (MEMTEST)
    magdiag();
#endif

    return;
}

