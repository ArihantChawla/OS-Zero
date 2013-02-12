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
#define NALLOC 1024

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

//    fprintf(stderr, "FLAGS: %lx\n", mag->flg);
#if (MAGBITMAP)
    fprintf(stderr, "BASE: %lx\n", (unsigned long)mag->base);
#endif
    fprintf(stderr, "N: %ld\n", mag->n);
    fprintf(stderr, "NDX: %ld\n", mag->ndx);
    fprintf(stderr, "BKT: %ld\n", mag->bkt);
    fprintf(stderr, "PREV: %p\n", mag->prev);
    fprintf(stderr, "NEXT: %p\n", mag->next);
#if (MAGBITMAP)
    fprintf(stderr, "BITMAP:");
    fprintf(stderr, " %lx", mag->bmap[0]);
    for (ul = 1 ; ul < (mag->n >> 3) ; ul++) {
        fprintf(stderr, " %lx", mag->bmap[ul]);
    }
    fprintf(stderr, "\n");
#endif    
    fprintf(stderr, "STACK:");
    for (ul = 0 ; ul < mag->n ; ul++) {
        fprintf(stderr, " %p", mag->ptab[ul]);
    }
    fprintf(stderr, "\n");

    return;
}

void
magdiag(void)
{
    struct maghdr *mag1;
    unsigned long  l;

    for (l = MAGMINLOG2 ; l < SLABMINLOG2 ; l++) {
        maglkq(magvirtlktab, l);
        mag1 = magvirttab[l];
        while (mag1) {
            if (mag1->ndx >= mag1->n) {
                fprintf(stderr, "too big index(%ld) on free list %lu: %ld\n",
                        mag1->ndx, l, mag1->n);
                magprint(mag1);

                abort();
            }
            mag1 = mag1->next;
        }
        magunlkq(magvirtlktab, l);
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
        fprintf(stderr, "BKT %lu -", ul);
        while (hdr1) {
            fprintf(stderr, " %p ", slabgetadr(hdr1, slabvirthdrtab));
            hdr1 = slabgetnext(hdr1, slabvirthdrtab);
        }
        fprintf(stderr, "\n");
    }
//    diag();

    return;
}

void
diag(void)
{
    unsigned long   l;
    unsigned long   n;
    struct slabhdr *hdr1;
    struct slabhdr *hdr2;
    struct slabhdr *hdr3;

    for (l = SLABMINLOG2 ; l < PTRBITS ; l++) {
//        slablk(slabvirtlktab, l);
        n = 0;
        hdr1 = slabvirttab[l];
        if (hdr1) {
            fprintf(stderr, "BKT %lu: ", l);
            if (slabgetprev(hdr1, slabvirthdrtab)) {
                hdr2 = slabgetprev(hdr1, slabvirthdrtab);
                fprintf(stderr, "%p: prev set on head: %p (%p)\n",
                        slabgetadr(hdr1, slabvirthdrtab),
                        slabgetprev(hdr1, slabvirthdrtab),
                        slabgetadr(hdr2, slabvirthdrtab));
                
                abort();
            }
            while (hdr1) {
                fprintf(stderr, " %lu: %p (%p)", n, slabgetadr(hdr1, slabvirthdrtab), hdr1);
                if (slabgetadr(hdr1, slabvirthdrtab) == NULL) {
                    fprintf(stderr, "NULL item on list\n");
                    
                    abort();
                }
                if (slabgetbkt(hdr1) != l) {
                    fprintf(stderr, "%p: invalid bkt %lu (%lu)\n",
                            slabgetadr(hdr1, slabvirthdrtab),
                            slabgetbkt(hdr1),
                        l);
                    
                    abort();
                }
                hdr2 = slabgetnext(hdr1, slabvirthdrtab);
                fprintf(stderr, " %lu: %p (%p)", n + 1, slabgetadr(hdr2, slabvirthdrtab), hdr2);
                if (hdr2) {
                    if (hdr1 == hdr2) {
                        fprintf(stderr, "%p: next is self\n",
                                slabgetadr(hdr1, slabvirthdrtab));
                    }
                    if (slabgetprev(hdr2, slabvirthdrtab) != hdr1) {
                        hdr3 = slabgetprev(hdr2, slabvirthdrtab);
                        fprintf(stderr, " %p: invalid prev %p (%p)\n",
                            slabgetadr(hdr2, slabvirthdrtab),
                                slabgetadr(hdr3, slabvirthdrtab),
                                slabgetadr(hdr1, slabvirthdrtab));
                        
                        abort();
                    }
                    if (slabgetnext(hdr1, slabvirthdrtab) != hdr2) {
                        hdr3 = slabgetnext(hdr1, slabvirthdrtab);
                        fprintf(stderr, " %p: invalid next %p (%p)\n",
                                slabgetadr(hdr1, slabvirthdrtab),
                                slabgetadr(hdr3, slabvirthdrtab),
                                slabgetadr(hdr2, slabvirthdrtab));
                        
                        abort();
                    }
                }
                hdr1 = hdr2;
            }
        }
//        slabunlk(slabvirtlktab, l);
        n++;
        fprintf(stderr, "%lu \n", n);
    }
}

void *
test(void *dummy)
{
    long   l;
    long  *alloctab = malloc(NALLOC * sizeof(long));
    void **ptrtab = malloc(NALLOC * sizeof(void *));

    for ( ; ; ) {
        for (l = 0 ; l < NALLOC ; l++) {
//            alloctab[l] = MAGMIN + (rand() & (4 * MAGMIN - 1));
            alloctab[l] = rand() & (8 * SLABMIN - 1);
        }
        for (l = 0 ; l < NALLOC ; l++) {
//            fprintf(stderr, "ALLOC: %lu - ", (unsigned long)alloctab[l]);
            ptrtab[l] = memalloc(alloctab[l], MEMZERO);
//            fprintf(stderr, "%p\n", ptrtab[l]);
        }
        l = NALLOC;
        while (l--) {
//            fprintf(stderr, "FREE: %lx\n", (unsigned long)ptrtab[l]);
            kfree((void *)ptrtab[l]);
        }
        slabprint();
    }

    return NULL;
}

#if 0
void *
thrtest(void *dummy)
{
    long   l;
    long   sz;
    void **ptrtab = calloc(NALLOC, sizeof(void *));

    while (1) {
        l = rand() & (NALLOC - 1);
        sz = rand() & (8 * SLABMIN - 1);
        ptrtab[l] = memalloc(sz, MEMZERO);
        if (ptrtab[l]) {
            kfree(ptrtab[l]);
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

    fprintf(stderr, "PTRBITS == %d\n", PTRBITS);
    fprintf(stderr, "MEMPID == %d\n", MEMPID);
    fprintf(stderr, "MALLOC: %p\n", base);
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

