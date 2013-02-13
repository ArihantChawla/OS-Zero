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

#define NTHR   4
#define NALLOC 128

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

extern struct maghdr  *magvirttab[PTRBITS];
extern struct slabhdr *slabvirttab[PTRBITS];
extern long            magvirtlktab[PTRBITS];
extern long            slabvirtlktab[PTRBITS];
extern unsigned long   slabvirtbase;
extern struct slabhdr *slabvirthdrtab;

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
    printf(" %x", mag->bmap[0]);
    for (ul = 1 ; ul < (mag->n >> 3) ; ul++) {
        printf(" %x", mag->bmap[ul]);
    }
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
    unsigned long  l;

    for (l = MAGMINLOG2 ; l < SLABMINLOG2 ; l++) {
#if 0
        (maglkq(magvirtlktab, l);
#endif
        if (mtxtrylk(&magvirtlktab[l], MEMPID)) {
            mag1 = magvirttab[l];
            while (mag1) {
                if (mag1->ndx >= mag1->n) {
                    printf("too big index(%ld) on free list %lu: %ld\n",
                            mag1->ndx, l, mag1->n);
                    magprint(mag1);
                    
                    abort();
                }
                mag1 = mag1->next;
            }
            mtxunlk(&magvirtlktab[l], MEMPID);
        }
#if 0
        magunlkq(magvirtlktab, l);
#endif
    }

    return;
}

void
slabprint(void)
{
    unsigned long ul;
    struct slabhdr *hdr1;

    for (ul = 0 ; ul < PTRBITS ; ul++) {
        hdr1 = slabvirttab[ul];
        printf("BKT %lu -", ul);
        while (hdr1) {
            printf(" %p ", slabgetadr(hdr1, slabvirthdrtab));
            hdr1 = slabgetnext(hdr1, slabvirthdrtab);
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
//        slablk(slabvirtlktab, bkt);
        n = 0;
        hdr1 = slabvirttab[bkt];
        if (hdr1) {
            printf("BKT %lu: ", bkt);
            if (slabgetprev(hdr1, slabvirthdrtab)) {
                hdr2 = slabgetprev(hdr1, slabvirthdrtab);
                printf("%p: prev set on head: %p (%p)\n",
                        slabgetadr(hdr1, slabvirthdrtab),
                        slabgetprev(hdr1, slabvirthdrtab),
                        slabgetadr(hdr2, slabvirthdrtab));
                
                abort();
            }
            while (hdr1) {
                printf(" %lu: %p (%p)", n, slabgetadr(hdr1, slabvirthdrtab), hdr1);
                if (slabgetadr(hdr1, slabvirthdrtab) == NULL) {
                    printf("NULL item on list\n");
                    
                    abort();
                }
                if (slabgetbkt(hdr1) != bkt) {
                    printf("%p: invalid bkt %lu (%lu)\n",
                            slabgetadr(hdr1, slabvirthdrtab),
                            slabgetbkt(hdr1),
                            bkt);
                    
                    abort();
                }
                hdr2 = slabgetnext(hdr1, slabvirthdrtab);
                printf(" %lu: %p (%p)", n + 1, slabgetadr(hdr2, slabvirthdrtab), hdr2);
                if (hdr2) {
                    if (hdr1 == hdr2) {
                        printf("%p: next is self\n",
                                slabgetadr(hdr1, slabvirthdrtab));
                    }
                    if (slabgetprev(hdr2, slabvirthdrtab) != hdr1) {
                        hdr3 = slabgetprev(hdr2, slabvirthdrtab);
                        printf(" %p: invalid prev %p(%ld) (%p)\n",
                                slabgetadr(hdr2, slabvirthdrtab),
                                slabgetadr(hdr3, slabvirthdrtab),
                                slabgetbkt(hdr2),
                                slabgetadr(hdr1, slabvirthdrtab));
                        
                        abort();
                    }
                    if (slabgetnext(hdr1, slabvirthdrtab) != hdr2) {
                        hdr3 = slabgetnext(hdr1, slabvirthdrtab);
                        printf(" %p: invalid next %p (%p)\n",
                                slabgetadr(hdr1, slabvirthdrtab),
                                slabgetadr(hdr3, slabvirthdrtab),
                                slabgetadr(hdr2, slabvirthdrtab));
                        
                        abort();
                    }
                }
                hdr1 = hdr2;
            }
        }
//        slabunlk(slabvirtlktab, bkt);
        n++;
        printf("%lu \n", n);
    }
}

void *
test(void *dummy)
{
    long   l;
    void **ptrtab = malloc(NALLOC * sizeof(void *));

    for ( ; ; ) {
        for (l = 0 ; l < NALLOC ; l++) {
            ptrtab[l] = memalloc(rand() & (8 * SLABMIN - 1), MEMZERO);
        }
        l = NALLOC;
        while (l--) {
            if (ptrtab[l]) {
                kfree((void *)ptrtab[l]);
            }
        }
        slabprint();
    }

    return NULL;
}

#if 0
void *
test(void *dummy)
{
    long   l;
    long   sz;
    void **ptrtab = calloc(NALLOC, sizeof(void *));

    while (1) {
        l = rand() & (NALLOC - 1);
        sz = rand() & (8 * SLABMIN - 1);
        ptrtab[l] = memalloc(sz, MEMZERO);
        diag();
        printf("PTR: %p\n", ptrtab[l]);
        if (ptrtab[l]) {
            kfree(ptrtab[l]);
            diag();
        }
    }

    return NULL;
}
#endif

int
main(int argc, char *argv[])
{
#if (MTSAFE)
    long  n = NTHR;
#endif
    void *base = memalign(SLABMIN, 1024 * 1024 * 1024);

    printf("PTRBITS == %d\n", PTRBITS);
    printf("MEMPID == %d\n", MEMPID);
    printf("MALLOC: %p\n", base);
    bzero(base, 1024 * 1024 * 1024);
    slabinit((unsigned long)base, 1024 * 1024 * 1024);
    slabprint();
#if (MTSAFE)
    while (n--) {
        pthread_create(&thrtab[n], NULL, test, NULL);
    }
    while (1) {
//        diag();
        ;
    }
#else
    test(NULL);
    while (1) {
        ;
    }
#endif
}

