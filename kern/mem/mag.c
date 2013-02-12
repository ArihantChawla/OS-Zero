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
long           magvirtlktab[PTRBITS];
struct maghdr *magvirthdrtab;
#if (!MAGBITMAP)
uint8_t       *magvirtbitmap;
#endif

void *
memalloc(unsigned long nb, long flg)
{
    void          *ptr = NULL;
    unsigned long  sz = 0;
    unsigned long  bkt;
    struct maghdr *mag;
    uint8_t       *u8ptr;
    unsigned long  l;
    unsigned long  n;

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
            mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
            bzero(mag, sizeof(struct maghdr));
#if (MAGBITMAP)
            mag->base = (uintptr_t)ptr;
#endif
            mag->n = 1;
            mag->ndx = 1;
        }
    } else {
        maglkq(magvirtlktab, bkt);
        mag = magvirttab[bkt];
        if (mag) {
            ptr = magpop(mag);
            if (magfull(mag)) {
                if (mag->next) {
                    mag->next->prev = NULL;
                }
                magvirttab[bkt] = mag->next;
            }
        } else {
            sz = 1UL << bkt;
            ptr = u8ptr = slaballoc(slabvirttab, slabvirthdrtab, sz, flg);
            if (ptr) {
                n = 1UL << (SLABMINLOG2 - bkt);
                mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
                bzero(mag, sizeof(struct maghdr));
#if (MAGBITMAP)
                mag->base = (uintptr_t)ptr;
#endif
                mag->n = n;
                mag->bkt = bkt;
#if (MEMTEST)
//            printf("INIT: %ld items:", n);
#endif
                for (l = 0 ; l < n ; l++) {
                    mag->ptab[l] = u8ptr;
#if (MEMTEST)
//                printf(" %p", u8ptr);
#endif
                    u8ptr += sz;
                }
#if (MEMTEST)
//            printf("\n");
#endif
//            ptr = magpop(mag);
                mag->ndx = 1;
                if (magvirttab[bkt]) {
                    magvirttab[bkt]->prev = mag;
                }
                mag->next = magvirttab[bkt];
                magvirttab[bkt] = mag;
            }
        }
        magunlkq(magvirtlktab, bkt);
    }
    if (ptr) {
#if (MAGBITMAP)
        if (bitset(mag->bmap, ((uintptr_t)ptr - mag->base) >> bkt)) {
            kprintf("duplicate allocation %p\n", ptr);
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
        if (flg & MEMZERO) {
            bzero(ptr, 1UL << bkt);
        }
    }
#if (MEMTEST)
//    printf("MAGPTR: %p\n", ptr);
#endif

    return ptr;
}

void
kfree(void *ptr)
{
    struct maghdr *mag = maggethdr(ptr, magvirthdrtab, slabvirtbase);
#if (MAGBITMAP)
    unsigned long  bkt = (mag) ? mag->bkt : 0;
#else
    unsigned long  bkt;
#endif

    if (!ptr) {

        return;
    }
#if (MAGBITMAP)
    if (!bitset(mag->bmap, ((uintptr_t)ptr - mag->base) >> bkt)) {
        kprintf("invalid free: %p\n", ptr);
        magprint(mag);

        panic();
    }
#else
    if (!bitset(magvirtbitmap, ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2)) {
        kprintf("invalid free: %p\n", ptr);

        panic();
    }
#endif
//    clrbit(magvirtbitmap, ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2);
    if (mag->n == 1 && mag->ndx == 1) {
        slabfree(slabvirttab, slabvirthdrtab, ptr);
    } else if (mag->n) {
        magpush(mag, ptr);
        if (magempty(mag)) {
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
#else
    clrbit(magvirtbitmap, ((uintptr_t)ptr - slabvirtbase) >> MAGMINLOG2);
#endif
#if (MEMTEST)
    magdiag();
#endif

    return;
}

