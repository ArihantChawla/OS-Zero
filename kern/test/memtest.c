#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/mem.h>
#include <kern/mem/mem.h>
#include <kern/mem/mag.h>
#include <kern/mem/slab.h>
#include <kern/unit/ia32/vm.h>

#define NALLOC 1024

unsigned long  vmnphyspages;

//uint32_t       pagetab[NPDE][NPTE] ALIGNED(PAGESIZE);
uintptr_t      alloctab[NALLOC];
void          *ptrtab[NALLOC];

extern struct maghdr  *magvirttab[PTRBITS];
extern struct slabhdr *slabvirttab[PTRBITS];
extern unsigned long   slabvirtbase;
extern struct slabhdr *slabvirthdrtab;

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
        mag1 = magvirttab[l];
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
        n = 0;
        hdr1 = slabvirttab[l];
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
//    slabinit(slabvirttab, slabvirthdrtab, PAGESIZE, 1024 * 1024);
    slabinit((unsigned long)base, 1024 * 1024 * 1024);
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

