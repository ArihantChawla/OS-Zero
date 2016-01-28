#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/malloc.h>
#include <kern/mem/mem.h>
#include <kern/mem/slab.h>
#include <kern/mem/pool.h>
#include <kern/unit/ia32/vm.h>

#define MEMTESTNTHR   16
#define MEMTESTNALLOC 256

unsigned long  vmnphyspages;

//uint32_t       pagetab[NPDE][NPTE] ALIGNED(PAGESIZE);
#if 0
uintptr_t      alloctab[MEMTESTNTHR][MEMTESTNALLOC];
#endif
#if 0
void          *ptrtab[MEMTESTNTHR * MEMTESTNALLOC];
#endif
volatile long  lktab[MEMTESTNTHR * MEMTESTNALLOC];
pthread_t      thrtab[MEMTESTNTHR];

extern struct mempool slabvirtzone;
extern struct mempool memvirtpool;

void
slabprint(struct memslab *slab)
{
    unsigned long ul;

    if (!slab) {

        return;
    }

//    printf("FLAGS: %lx\n", slab->flg);
#if (SLABBITMAP)
    printf("BASE: %lx\n", (unsigned long)slab->base);
#endif
    printf("N: %ld\n", slab->n);
    printf("NDX: %ld\n", slab->ndx);
    printf("BKT: %ld\n", slab->bkt);
    printf("PREV: %p\n", slab->prev);
    printf("NEXT: %p\n", slab->next);
#if (SLABBITMAP)
    printf("BITMAP:");
#if ((SLABLOG2 - MEMSLABMINLOG2) < (LONGSIZELOG2 + 3))
    printf(" %lx", slab->bmap);
#else
    printf(" %x", slab->bmap[0]);
    for (ul = 1 ; ul < (slab->n >> 3) ; ul++) {
        printf(" %x", slab->bmap[ul]);
    }
#endif
    printf("\n");
#endif    
    printf("STACK:");
    for (ul = 0 ; ul < slab->n ; ul++) {
        printf(" %p", slab->ptab[ul]);
    }
    printf("\n");

    return;
}

void
slabdiag(void)
{
    struct memslab *slab1;
    unsigned long  l;

    for (l = MEMSLABMINLOG2 ; l < MEMSLABMINLOG2 ; l++) {
#if 0
        slablkq(slabvirtzone.lktab, l);
#endif
        if (mtxtrylk(&slabvirtzone.tab[l].lk)) {
            slab1 = slabvirtzone.tab[l].list;
            while (slab1) {
                if (slab1->bkt != l) {
                    printf("invalid bkt(%ld) on free list %lu\n",
                           slab1->bkt, l);
                    slabprint(slab1);
                    
                    abort();
                }
                if (slab1->ndx >= slab1->n) {
                    printf("too big index(%ld) on free list %lu: %ld\n",
                            slab1->ndx, l, slab1->n);
                    slabprint(slab1);
                    
                    abort();
                }
                slab1 = slab1->next;
            }
            mtxunlk(&lktab[l]);
        }
    }

    return;
}

void
slabprintall(void)
{
    unsigned long ul;
    struct memslab *hdr1;

    for (ul = 0 ; ul < PTRBITS ; ul++) {
        hdr1 = slabvirtzone.tab[ul];
        printf("BKT %lu -", ul);
        while (hdr1) {
            printf(" %p ", memslabgetadr(hdr1, &slabvirtzone));
            hdr1 = memslabgetnext(hdr1, &slabvirtzone);
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
    struct memslab *hdr1;
    struct memslab *hdr2;
    struct memslab *hdr3;

    for (bkt = MEMSLABMINLOG2 ; bkt < PTRBITS ; bkt++) {
        mtxlk(&lktab[bkt]);
        n = 0;
        hdr1 = slabvirtzone.tab[bkt];
        if (hdr1) {
            printf("BKT %lu: ", bkt);
            if (memslabgetprev(hdr1, slabvirthdrtab)) {
                hdr2 = memslabgetprev(hdr1, slabvirthdrtab);
                printf("%p: prev set on head: %p (%p)\n",
                        memslabgetadr(hdr1, &slabvirtzone),
                        memslabgetprev(hdr1, &slabvirtzone),
                        memslabgetadr(hdr2, &slabvirtzone));
                
                abort();
            }
            while (hdr1) {
                printf(" %lu: %p (%p)", n, memslabgetadr(hdr1, &slabvirtzone), hdr1);
                if (memslabgetadr(hdr1, &slabvirtzone) == NULL) {
                    printf("NULL item on list\n");
                    
                    abort();
                }
                if (memslabgetbkt(hdr1) != bkt) {
                    printf("%p: invalid bkt %lu (%lu)\n",
                            memslabgetadr(hdr1, &slabvirtzone),
                            memslabgetbkt(hdr1),
                            bkt);
                    slabprint(hdr1);
                    
                    abort();
                }
                hdr2 = memslabgetnext(hdr1, &slabvirtzone);
                if (hdr2) {
                    printf(" %lu: %p (%p)", n + 1, memslabgetadr(hdr2, &slabvirtzone), hdr2);
                    if (hdr1 == hdr2) {
                        printf("%p: next is self\n",
                                memslabgetadr(hdr1, &slabvirtzone));
                    }
                    if (memslabgetprev(hdr2, &slabvirtzone) != hdr1) {
                        hdr3 = memslabgetprev(hdr2, &slabvirtzone);
                        printf(" %p: invalid prev %p(%ld) (%p)\n",
                                memslabgetadr(hdr2, &slabvirtzone),
                                memslabgetadr(hdr3, &slabvirtzone),
                                memslabgetbkt(hdr2),
                                memslabgetadr(hdr1, &slabvirtzone));
                        
                        abort();
                    }
                    if (memslabgetnext(hdr1, &slabvirtzone) != hdr2) {
                        hdr3 = memslabgetnext(hdr1, &slabvirtzone);
                        printf(" %p: invalid next %p (%p)\n",
                                memslabgetadr(hdr1, &slabvirtzone),
                                memslabgetadr(hdr3, &slabvirtzone),
                                memslabgetadr(hdr2, &slabvirtzone));
                        
                        abort();
                    }
                }
                hdr1 = hdr2;
            }
        }
        mtxunlk(&lktab[bkt]);
        n++;
        printf("%lu \n", n);
    }
}

void *
test(void *dummy)
{
    long   l;
    void **ptrtab = calloc(MEMTESTNALLOC, sizeof(void *));

    for ( ; ; ) {
        for (l = 0 ; l < MEMTESTNALLOC ; l++) {
            ptrtab[l] = memalloc(rand() & (8 * MEMSLABMIN - 1), MEMZERO);
#if (MEMPRINT)
            if (!ptrtab[l]) {
                fprintf(stderr, "failed to allocate memory\n");
            }
#endif
        }
        l = MEMTESTNALLOC;
        while (l--) {
            if (ptrtab[l]) {
                kfree((void *)ptrtab[l]);
                ptrtab[l] = NULL;
            }
        }
#if (MEMPRINT)
        slabprintall();
#endif
    }

    return NULL;
}

int
main(int argc, char *argv[])
{
    long  n = MEMTESTNTHR;
    void *base = memalign(MEMSLABMIN, 256 * 1024 * 1024);

    printf("PTRBITS == %d\n", PTRBITS);
//    printf("MEMPID == %d\n", MEMPID);
    printf("MALLOC: %p\n", base);
    bzero(base, 256 * 1024 * 1024);
    slabinit(&slabvirtzone, (unsigned long)base, 256 * 1024 * 1024);
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

