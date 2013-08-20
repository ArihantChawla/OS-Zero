#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/mem.h>
#include <kern/mem/mem.h>
#include <kern/mem/mag.h>
#include <kern/mem/slab.h>
#include <kern/unit/ia32/vm.h>

#define NTHR   16
#define NALLOC 256

unsigned long  vmnphyspages;

//uint32_t       pagetab[NPDE][NPTE] ALIGNED(PAGESIZE);
#if 0
uintptr_t      alloctab[NTHR][NALLOC];
#endif
#if 0
void          *ptrtab[NTHR * NALLOC];
#endif
volatile long  lktab[NTHR * NALLOC];
pthread_t      thrtab[NTHR];

extern struct memzone slabvirtzone;
extern struct memzone magvirtzone;

void
magprint(struct maghdr *mag)
{
    unsigned long ul;

    if (!mag) {

        return;
    }

//    printf("FLAGS: %lx\n", mag->flg);
#if (MAGBITMAP)
    printf("BASE: %lx\n", (unsigned long)mag->base);
#endif
    printf("N: %ld\n", mag->n);
    printf("NDX: %ld\n", mag->ndx);
    printf("BKT: %ld\n", mag->bkt);
    printf("PREV: %p\n", mag->prev);
    printf("NEXT: %p\n", mag->next);
#if (MAGBITMAP)
    printf("BITMAP:");
#if ((SLABLOG2 - MAGMINLOG2) < (LONGSIZELOG2 + 3))
    printf(" %lx", mag->bmap);
#else
    printf(" %x", mag->bmap[0]);
    for (ul = 1 ; ul < (mag->n >> 3) ; ul++) {
        printf(" %x", mag->bmap[ul]);
    }
#endif
    printf("\n");
#endif    
    printf("STACK:");
    for (ul = 0 ; ul < mag->n ; ul++) {
        printf(" %p", mag->ptab[ul]);
    }
    printf("\n");

    return;
}

void
magdiag(void)
{
    struct maghdr *mag1;
    volatile long *lktab = magvirtzone.lktab;
    unsigned long  l;

    for (l = MAGMINLOG2 ; l < SLABMINLOG2 ; l++) {
#if 0
        maglkq(magvirtzone.lktab, l);
#endif
        if (mtxtrylk(&lktab[l])) {
            mag1 = magvirtzone.tab[l];
            while (mag1) {
                if (mag1->bkt != l) {
                    printf("invalid bkt(%ld) on free list %lu\n",
                           mag1->bkt, l);
                    magprint(mag1);
                    
                    abort();
                }
                if (mag1->ndx >= mag1->n) {
                    printf("too big index(%ld) on free list %lu: %ld\n",
                            mag1->ndx, l, mag1->n);
                    magprint(mag1);
                    
                    abort();
                }
                mag1 = mag1->next;
            }
            mtxunlk(&lktab[l]);
        }
    }

    return;
}

void
slabprint(struct slabhdr *hdr)
{
    printf("NFO: %lx\n", hdr->nfo);
    printf("BKT: %lu\n", slabgetbkt(hdr));

    return;
}

void
slabprintall(void)
{
    unsigned long ul;
    struct slabhdr *hdr1;

    for (ul = 0 ; ul < PTRBITS ; ul++) {
        hdr1 = slabvirtzone.tab[ul];
        printf("BKT %lu -", ul);
        while (hdr1) {
            printf(" %p ", slabgetadr(hdr1, &slabvirtzone));
            hdr1 = slabgetnext(hdr1, &slabvirtzone);
        }
        printf("\n");
    }
//    diag();

    return;
}

void
diag(void)
{
    unsigned long   bkt;
    unsigned long   n;
    struct slabhdr *hdr1;
    struct slabhdr *hdr2;
    struct slabhdr *hdr3;

    for (bkt = SLABMINLOG2 ; bkt < PTRBITS ; bkt++) {
        mtxlk(&lktab[bkt]);
        n = 0;
        hdr1 = slabvirtzone.tab[bkt];
        if (hdr1) {
            printf("BKT %lu: ", bkt);
            if (slabgetprev(hdr1, slabvirthdrtab)) {
                hdr2 = slabgetprev(hdr1, slabvirthdrtab);
                printf("%p: prev set on head: %p (%p)\n",
                        slabgetadr(hdr1, &slabvirtzone),
                        slabgetprev(hdr1, &slabvirtzone),
                        slabgetadr(hdr2, &slabvirtzone));
                
                abort();
            }
            while (hdr1) {
                printf(" %lu: %p (%p)", n, slabgetadr(hdr1, &slabvirtzone), hdr1);
                if (slabgetadr(hdr1, &slabvirtzone) == NULL) {
                    printf("NULL item on list\n");
                    
                    abort();
                }
                if (slabgetbkt(hdr1) != bkt) {
                    printf("%p: invalid bkt %lu (%lu)\n",
                            slabgetadr(hdr1, &slabvirtzone),
                            slabgetbkt(hdr1),
                            bkt);
                    slabprint(hdr1);
                    
                    abort();
                }
                hdr2 = slabgetnext(hdr1, &slabvirtzone);
                if (hdr2) {
                    printf(" %lu: %p (%p)", n + 1, slabgetadr(hdr2, &slabvirtzone), hdr2);
                    if (hdr1 == hdr2) {
                        printf("%p: next is self\n",
                                slabgetadr(hdr1, &slabvirtzone));
                    }
                    if (slabgetprev(hdr2, &slabvirtzone) != hdr1) {
                        hdr3 = slabgetprev(hdr2, &slabvirtzone);
                        printf(" %p: invalid prev %p(%ld) (%p)\n",
                                slabgetadr(hdr2, &slabvirtzone),
                                slabgetadr(hdr3, &slabvirtzone),
                                slabgetbkt(hdr2),
                                slabgetadr(hdr1, &slabvirtzone));
                        
                        abort();
                    }
                    if (slabgetnext(hdr1, &slabvirtzone) != hdr2) {
                        hdr3 = slabgetnext(hdr1, &slabvirtzone);
                        printf(" %p: invalid next %p (%p)\n",
                                slabgetadr(hdr1, &slabvirtzone),
                                slabgetadr(hdr3, &slabvirtzone),
                                slabgetadr(hdr2, &slabvirtzone));
                        
                        abort();
                    }
                }
                hdr1 = hdr2;
            }
        }
        mtxlk(&lktab[bkt]);
        n++;
        printf("%lu \n", n);
    }
}

void *
test(void *dummy)
{
    long   l;
    void **ptrtab = calloc(NALLOC, sizeof(void *));

    for ( ; ; ) {
        for (l = 0 ; l < NALLOC ; l++) {
//            ptrtab[l] = memalloc(rand() & (8 * SLABMIN - 1), MEMZERO);
            ptrtab[l] = memalloc(rand() & (8 * SLABMIN - 1), MEMZERO);
        }
        l = NALLOC;
        while (l--) {
            if (ptrtab[l]) {
                kfree((void *)ptrtab[l]);
                ptrtab[l] = NULL;
            }
        }
//        slabprintall();
    }

    return NULL;
}

int
main(int argc, char *argv[])
{
    long  n = NTHR;
    void *base = memalign(SLABMIN, 1024 * 1024 * 1024);

    printf("PTRBITS == %d\n", PTRBITS);
    printf("MEMPID == %d\n", MEMPID);
    printf("MALLOC: %p\n", base);
    bzero(base, 1024 * 1024 * 1024);
    slabinit(&slabvirtzone, (unsigned long)base, 1024 * 1024 * 1024);
    slabprintall();
    while (n--) {
        pthread_create(&thrtab[n], NULL, test, NULL);
        pthread_detach(thrtab[n]);
    }
    while (1) {
//        diag();
        ;
    }

    /* NOTREACHED */
    exit(0);
}

