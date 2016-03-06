#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zero/mtx.h>
#include <kern/util.h>
#include <kern/mem/mem.h>
#include <kern/mem/slab.h>
#include <kern/mem/mag.h>
#include <kern/mem/bkt.h>
#include <kern/mem/vm.h>

#define MEMDIAG 0

extern size_t         npagefree;
extern struct mempool memvirtpool;

struct mempool        memphyspool ALIGNED(PAGESIZE);
static volatile long  memphyspoollk;
/*
 * zero slab allocator
 * -------------------
 * - slabs are power-of-two-sizes
 * - slabs are combined to and split from bigger ones on demand;
 *   free regions are kept as big as possible.
 */

#if !(__KERNEL__) && (MEMDIAG)
void
memdiag(struct mempool *pool)
{
    long            bkt;
    struct memslab *slab1;
    struct memslab *slab2;
    struct memslab *null = NULL;

    mtxlk(&memphyspoollk);
    for (bkt = 0 ; bkt < PTRBITS ; bkt++) {
            slab1 = (struct memslab **)pool->tab[bkt];
            if (slab1) {
                if (memgetprev(slab1)) {
#if (__KERNEL__)
                    fprintf(stderr, "DIAG: non-NULL prev on head of list %ld\n",
                            bkt);
#endif
                    memprint(slab1);
                    
                    *null = *slab1;
                }
                if (memslabgetbkt(slab1) != bkt) {
#if (__KERNEL__)
                    fprintf(stderr, "DIAG: invalid bucket ID on head of list %ld\n",
                            bkt);
#endif
                    memprint(slab1);
                    
                    
                    *null = *slab1;
                }
                slab2 = memgetnext(slab1);
                while (slab2) {
                    if (memgetprev(slab2) != slab1) {
#if (__KERNEL__)
                        fprintf(stderr, "DIAG: invalid prev on list %ld\n",
                                bkt);
#endif
                        memprint(slab1);
                        
                        
                        *null = *slab1;
                    }
                    if (memslabgetbkt(slab2) != bkt) {
#if (__KERNEL__)
                        fprintf(stderr, "DIAG: invalid bucket ID on list %ld\n",
                                bkt);
#endif
                        memprint(slab1);
                        
                        
                        *null = *slab1;
                    }
                    slab1 = slab2;
                    slab2 = memgetnext(slab2);
                }
            }
        }
    }
    mtxunlk(&memphyspoollk);

    return;
}
#endif

/*
 * combine slab with bigger [previous- or next-address] ones if possible.
 */
unsigned long
slabcomb(struct mempool *pool, struct memslab *slab)
{
    struct memslab **slabtab = (struct memslab **)pool->tab;
    struct memslab  *blktab = pool->blktab;
    unsigned long    nblk = pool->nblk;
    unsigned long    bkt = memslabgetbkt(slab);
    unsigned long    bkt1 = bkt;
    unsigned long    bkt2 = bkt1;
    unsigned long    ret  = 0;
    long             prev = 1;
    long             next = 1;
    intptr_t         ofs = 1UL << (bkt1 - MEMMINSHIFT);
    struct memslab  *slab1;
    struct memslab  *slab2;
    struct memslab  *slab3;
    struct memslab  *slab4;

    slab1 = slab;
    while ((prev) || (next)) {
        prev ^= prev;
        next ^= next;
        if (slab - (struct memslab *)blktab >= ofs) {
            slab1 = slab - ofs;
            bkt2 = memslabgetbkt(slab1);
            if (bkt2 == bkt1 && memslabisfree(slab1)) {
                prev++;
                ret++;
                slab3 = memslabgetprev(slab1);
                slab4 = memslabgetnext(slab1);
                if ((slab3) && (slab4)) {
                    memslabsetnext(slab3, slab4);
                    memslabsetprev(slab4, slab3);
                } else if (slab3) {
                    memslabsetnext(slab3, slab4); // NULL
                } else if (slab4) {
                    memslabsetprev(slab4, slab3); // NULL
                    slabtab[bkt1] = slab4;
                } else {
                    slabtab[bkt1] = NULL;
                }
                memslabclrinfo(slab);
                memslabclrlink(slab);
//                bkt2++;
                bkt1++;
                ofs <<= 1;
            }
        }
        if (!slab1) {
            slab1 = slab;
        }
        if (slab1 + ofs < (struct memslab *)blktab + nblk) {
            slab2 = slab1 + ofs;
            bkt2 = memslabgetbkt(slab2);
            if (bkt2 == bkt1 && memslabisfree(slab2)) {
                next++;
                ret++;
                slab3 = memslabgetprev(slab2);
                slab4 = memslabgetnext(slab2);
                if ((slab3) && (slab4)) {
                    memslabsetnext(slab3, slab4);
                    memslabsetprev(slab4, slab3);
                } else if (slab3) {
                    memslabsetnext(slab3, slab4); // NULL;
                } else if (slab4) {
                    memslabsetprev(slab4, slab3); // NULL
                    slabtab[bkt1] = slab4;
                } else {
                    slabtab[bkt1] = NULL;
                }
                memslabclrinfo(slab2);
                memslabclrlink(slab2);
                bkt2++;
                memslabclrinfo(slab1);
                memslabclrlink(slab1);
                memslabsetbkt(slab1, bkt2);
                memslabsetfree(slab1);
                bkt1 = bkt2;
                ofs <<= 1;
                slab1 = NULL;
            }
        }
        if (slab1) {
            slab = slab1;
        }
    }
    if (ret) {
        memslabclrinfo(slab);
        memslabclrlink(slab);
        memslabsetbkt(slab, bkt1);
        memslabsetfree(slab);
        if (slabtab[bkt1]) {
            memslabsetprev(slabtab[bkt1], slab);
            memslabsetnext(slab, slabtab[bkt1]);
        }
        slabtab[bkt1] = slab;
    }
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif

    return ret;
}

/*
 * - split slab into smaller ones to satisfy allocation request
 * - split of N to M gives us one free slab in each of M to N-1 and one to
 *   allocate in M
 * - caller has locked the bucket dest
 */    
void
slabsplit(struct mempool *pool, struct memslab *slab, unsigned long dest)
{
    struct memslab **blktab = (struct memslab **)pool->tab;
    unsigned long    bkt = memslabgetbkt(slab);
    uint8_t         *ptr = memgetadr(slab, pool);
    struct memslab  *slab1;
    size_t           sz = 1UL << bkt;

    ptr += 1UL << dest;
    while (--bkt >= dest) {
        sz >>= 1;
        ptr -= sz;
        slab1 = memgethdr(ptr, pool);
        memslabclrinfo(slab1);
        memslabclrlink(slab1);
        memslabsetbkt(slab1, bkt);
        memslabsetfree(slab1);
        if (blktab[bkt]) {
            memslabsetprev(blktab[bkt], slab1);
            memslabsetnext(slab1, blktab[bkt]);
        }
        blktab[bkt] = slab1;
    }
//    slab1 = memgethdr(ptr);
    memslabclrinfo(slab);
    memslabclrlink(slab);
    memslabsetbkt(slab, dest);
    memslabsetfree(slab);
    if (blktab[dest]) {
        memslabsetprev(blktab[dest], slab);
        memslabsetnext(slab, blktab[dest]);
    }
    blktab[dest] = slab;
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif
        
    return;
}

/*
 * allocate slab; split from larger ones if necessary.
 */
void *
slaballoc(struct mempool *pool, unsigned long nb, unsigned long flg)
{
    struct memslab **blktab = (struct memslab **)pool->tab;
    unsigned long    bkt1 = memcalcbkt(nb);
    unsigned long    bkt2 = bkt1;
    uint8_t         *ptr = NULL;
    struct memslab  *slab1;
    struct memslab  *slab2;

    mtxlk(&memphyspoollk);
    slab1 = blktab[bkt1];
    if (!slab1) {
        while (!slab1 && ++bkt2 < PTRBITS) {
            slab1 = blktab[bkt2];
            if (slab1) {
                slab2 = memslabgetnext(slab1);
                if (slab2) {
                    memslabclrprev(slab2);
                }
                blktab[bkt2] = slab2;
                memslabclrlink(slab1);
                slabsplit(pool, slab1, bkt1);
                slab1 = blktab[bkt1];
            }
        }
    }
    if (slab1) {
        slab2 = memslabgetnext(slab1);
        if (slab2) {
            memslabclrprev(slab2);
        }
        blktab[bkt1] = slab2;
        memslabclrfree(slab1);
        memslabclrlink(slab1);
        memslabsetflg(slab1, flg);
        ptr = memgetadr(slab1, pool);
    }
    mtxunlk(&memphyspoollk);
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif

    return ptr;
}

/*
 * free slab; combine to form bigger ones if possible.
 */
void
slabfree(struct mempool *physpool, void *ptr)
{
    struct memslab **blktab = (struct memslab **)physpool->tab;
    struct memslab  *slab1 = memgethdr(ptr, physpool);
    unsigned long    bkt = memslabgetbkt(slab1);

    mtxlk(&memphyspoollk);
#if (!MEMTEST)
    vmfreephys(ptr, 1UL << bkt);
#endif
    memslabsetfree(slab1);
    if (!slabcomb(physpool, slab1)) {
        memslabclrlink(slab1);
        if (blktab[bkt]) {
            memslabsetprev(blktab[bkt], slab1);
        }
        memslabsetnext(slab1, blktab[bkt]);
        blktab[bkt] = slab1;
    }
    mtxunlk(&memphyspoollk);
#if (__KERNEL__ && (MEMDIAG))
    memdiag(physpool);
#endif

    return;
}

