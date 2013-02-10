#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/mem.h>
#include <kern/mem/mem.h>
#include <kern/mem/slab.h>
#include <kern/mem/mag.h>
#include <kern/mem/slab32.h>
#include <kern/unit/ia32/vm.h>

#define NALLOC 1024

unsigned long  vmnphyspages;

//uint32_t       pagetab[NPDE][NPTE] ALIGNED(PAGESIZE);
uintptr_t      alloctab[NALLOC];
void          *ptrtab[NALLOC];

extern struct maghdr *freemagtab[PTRBITS];

void
magprint(struct maghdr *mag)
{
    unsigned long ul;

//    fprintf(stderr, "FLAGS: %lx\n", mag->flg);
    fprintf(stderr, "N: %ld\n", mag->n);
    fprintf(stderr, "NDX: %ld\n", mag->ndx);
    fprintf(stderr, "PREV: %p\n", mag->prev);
    fprintf(stderr, "NEXT: %p\n", mag->next);

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
        mag1 = freemagtab[l];
        while (mag1) {
            if (!mag1->ndx) {
                fprintf(stderr, "empty magazine on free list %lu\n", l);
                magprint(mag1);

                abort();
            }
            mag1 = mag1->next;
        }
    }

    return;
}

void
slabprint(void)
{
    unsigned long ul;
    struct slabhdr *hdr1;

    for (ul = 0 ; ul < PTRBITS ; ul++) {
        hdr1 = virtslabtab[ul];
        fprintf(stderr, "BKT %lu -", ul);
        while (hdr1) {
            fprintf(stderr, " %p ", slabadr(hdr1, virthdrtab));
            hdr1 = slabgetnext(hdr1, virthdrtab);
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
        n = 0;
        hdr1 = virtslabtab[l];
        fprintf(stderr, "BKT %lu: ", l);
        if (slabgetprev(hdr1, virthdrtab)) {
            hdr2 = slabgetprev(hdr1, virthdrtab);
            fprintf(stderr, "%p: prev set on head: %p (%p)\n",
                    slabadr(hdr1, virthdrtab),
                    slabgetprev(hdr1, virthdrtab),
                    slabadr(hdr2, virthdrtab));
            
            abort();
        }
        while (hdr1) {
            fprintf(stderr, " %lu: %p (%p)", n, slabadr(hdr1, virthdrtab), hdr1);
            if (slabadr(hdr1, virthdrtab) == NULL) {
                fprintf(stderr, "NULL item on list\n");

                abort();
            }
            if (slabgetbkt(hdr1) != l) {
                fprintf(stderr, "%p: invalid bkt %lu (%lu)\n",
                        slabadr(hdr1, virthdrtab),
                        slabgetbkt(hdr1), l);

                abort();
            }
            hdr2 = slabgetnext(hdr1, virthdrtab);
            fprintf(stderr, " %lu: %p (%p)", n + 1, slabadr(hdr2, virthdrtab), hdr2);
            if (hdr2) {
                if (hdr1 == hdr2) {
                    fprintf(stderr, "%p: next is self\n",
                            slabadr(hdr1, virthdrtab));
                }
                if (slabgetprev(hdr2, virthdrtab) != hdr1) {
                    hdr3 = slabgetprev(hdr2, virthdrtab);
                    fprintf(stderr, " %p: invalid prev %p (%p)\n",
                            slabadr(hdr2, virthdrtab),
                            slabadr(hdr3, virthdrtab),
                            slabadr(hdr1, virthdrtab));
                    
                    abort();
                }
                if (slabgetnext(hdr1, virthdrtab) != hdr2) {
                    hdr3 = slabgetnext(hdr1, virthdrtab);
                    fprintf(stderr, " %p: invalid next %p (%p)\n",
                            slabadr(hdr1, virthdrtab),
                            slabadr(hdr3, virthdrtab),
                            slabadr(hdr2, virthdrtab));
                    
                    abort();
                }
            }
            hdr1 = hdr2;
            n++;
        }
        fprintf(stderr, "%lu \n", n);
    }
}

int
main(int argc, char *argv[])
{
//    long  n = (3U * 1024 * 1024 * 1024) >> PAGESIZELOG2;
    long  l;
    void *base = memalign(SLABMIN, 1024 * 1024 * 1024);

//    pageinit(0, 3U * 1024 * 1024 * 1024);
    fprintf(stderr, "PTRBITS == %d\n", PTRBITS);
    fprintf(stderr, "MALLOC: %p\n", base);
    bzero(base, 1024 * 1024 * 1024);
//    slabinit(virtslabtab, virthdrtab, PAGESIZE, 1024 * 1024);
    slabinit(virtslabtab, virthdrtab, (unsigned long)base, 1024 * 1024 * 1024);
//    diag();
    slabprint();
    sleep(1);
    srand(1);
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
        diag();
        slabprint();
    }
#if 0
    ptr1 = memalloc(PAGESIZE, 0);
    fprintf(stderr, "PTR1: %p\n", ptr1);
    ptr2 = memalloc(PAGESIZE, 0);
    fprintf(stderr, "PTR2: %p\n", ptr2);
    kfree(ptr2);
    kfree(ptr1);
    slabprint();
#endif

    exit(0);
}

