#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <zero/mtx.h>
#include <kern/util.h>
#include <kern/mem/mem.h>
//#include <kern/mem/slab.h>
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
    long            bktid;
    struct memslab *slab1;
    struct memslab *slab2;
    struct memslab *null = NULL;

    mtxlk(&memphyspoollk);
    for (bktid = 0 ; bktid < PTRBITS ; bktid++) {
            slab1 = (struct memslab **)pool->tab[bktid];
            if (slab1) {
                if (memgetprev(slab1)) {
#if (__KERNEL__)
                    fprintf(stderr, "DIAG: non-NULL prev on head of list %ld\n",
                            bktid);
#endif
                    memprint(slab1);
                    
                    *null = *slab1;
                }
                if (memslabgetbkt(slab1) != bktid) {
#if (__KERNEL__)
                    fprintf(stderr, "DIAG: invalid bucket ID on head of list %ld\n",
                            bktid);
#endif
                    memprint(slab1);
                    
                    
                    *null = *slab1;
                }
                slab2 = memgetnext(slab1);
                while (slab2) {
                    if (memgetprev(slab2) != slab1) {
#if (__KERNEL__)
                        fprintf(stderr, "DIAG: invalid prev on list %ld\n",
                                bktid);
#endif
                        memprint(slab1);
                        
                        
                        *null = *slab1;
                    }
                    if (memslabgetbkt(slab2) != bktid) {
#if (__KERNEL__)
                        fprintf(stderr, "DIAG: invalid bucket ID on list %ld\n",
                                bktid);
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

#if 0

/*
 * combine slab with bigger [previous- or next-address] ones if possible.
 */
unsigned long
slabcomb(struct mempool *pool, struct memslab *slab)
{
    struct membkt  *bkt = pool->tab;
    unsigned long   nblk = pool->nblk;
    unsigned long   bktid = memslabgetbkt(slab);
    unsigned long   bktid1 = bktid;
    unsigned long   bktid2 = bktid1;
    unsigned long   ret  = 0;
    long            prev = 1;
    long            next = 1;
    intptr_t        ofs = 1UL << (bktid1 - MEMMINSHIFT);
    struct memslab *slab1;
    struct memslab *slab2;
    struct memslab *slab3;
    struct memslab *slab4;

    slab1 = slab;
    while ((prev) || (next)) {
        prev ^= prev;
        next ^= next;
        if (slab - (struct memslab *)pool->hdrtab >= ofs) {
            slab1 = slab - ofs;
            bktid2 = memslabgetbkt(slab1);
            if (bktid2 == bktid1 && memslabisfree(slab1)) {
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
                    bkt[bktid1].list = slab4;
                } else {
                    bkt[bktid1].list = NULL;
                }
                memslabclrinfo(slab);
                memslabclrlink(slab);
//                bktid2++;
                bktid1++;
                ofs <<= 1;
            }
        }
        if (!slab1) {
            slab1 = slab;
        }
        if (slab1 + ofs < (struct memslab *)pool->hdrtab + nblk) {
            slab2 = slab1 + ofs;
            bktid2 = memslabgetbkt(slab2);
            if (bktid2 == bktid1 && memslabisfree(slab2)) {
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
                    bkt[bktid1].list = slab4;
                } else {
                    bkt[bktid1].list = NULL;
                }
                memslabclrinfo(slab2);
                memslabclrlink(slab2);
                bktid2++;
                memslabclrinfo(slab1);
                memslabclrlink(slab1);
                memslabsetbkt(slab1, bktid2);
                memslabsetfree(slab1);
                bktid1 = bktid2;
                ofs <<= 1;
                slab1 = NULL;
            }
        }
        if (slab1) {
            slab = slab1;
        }
    }
    if (ret) {
        slab1 = bkt[bktid].list;
        memslabclrinfo(slab);
        memslabclrlink(slab);
        memslabsetbkt(slab, bktid1);
        memslabsetfree(slab);
        if (slab1) {
            memslabsetprev(slab1, slab);
            memslabsetnext(slab, slab1);
        }
        bkt[bktid1].list = slab;
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
    struct membkt  *bkt = pool->tab;
    unsigned long   bktid = memslabgetbkt(slab);
    uint8_t        *ptr = memgetadr(slab, pool);
    struct memslab *slab1;
    struct memslab *slab2;
    size_t          sz = 1UL << bktid;

    ptr += 1UL << bktid;
    while (--bktid >= dest) {
        sz >>= 1;
        ptr -= sz;
        slab1 = memgetslab(ptr, pool);
        slab2 = bkt[bktid].list;
        memslabclrinfo(slab1);
        memslabclrlink(slab1);
        memslabsetbkt(slab1, bktid);
        memslabsetfree(slab1);
        if (slab2) {
            memslabsetprev(slab2, slab1);
            memslabsetnext(slab1, slab2);
        }
        bkt[bktid].list = slab1;
    }
    slab1 = bkt[dest].list;
    memslabclrinfo(slab);
    memslabclrlink(slab);
    memslabsetbkt(slab, dest);
    memslabsetfree(slab);
    if (slab1) {
        memslabsetprev(slab1, slab);
        memslabsetnext(slab, slab1);
    }
    bkt[dest].list = slab;
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif
        
    return;
}

#endif /* 0 */

/*
 * combine slab with bigger [previous- or next-address] ones if possible.
 */
unsigned long
slabcomb(struct mempool *pool, struct memmag *mag)
{
    struct membkt *bkt = pool->tab;
    unsigned long  nblk = pool->nblk;
    unsigned long  bktid = memmaggetbkt(mag);
    unsigned long  bktid1 = bktid;
    unsigned long  bktid2 = bktid1;
    unsigned long  ret  = 0;
    long           prev = 1;
    long           next = 1;
    intptr_t       ofs = 1UL << (bktid1 - MEMMINSHIFT);
    struct memmag *mag1;
    struct memmag *mag2;
    struct memmag *mag3;
    struct memmag *mag4;

    mag1 = mag;
    while ((prev) || (next)) {
        prev ^= prev;
        next ^= next;
        if (mag - (struct memmag *)pool->hdrtab >= ofs) {
            mag1 = mag - ofs;
            bktid2 = memmaggetbkt(mag1);
            if (bktid2 == bktid1 && memmagisfree(mag1)) {
                prev++;
                ret++;
                mag3 = memmaggetprev(mag1);
                mag4 = memmaggetnext(mag1);
                if ((mag3) && (mag4)) {
                    memmagsetnext(mag3, mag4);
                    memmagsetprev(mag4, mag3);
                } else if (mag3) {
                    memmagsetnext(mag3, mag4); // NULL
                } else if (mag4) {
                    memmagsetprev(mag4, mag3); // NULL
                    bkt[bktid1].list = mag4;
                } else {
                    bkt[bktid1].list = NULL;
                }
                memmagclrinfo(mag);
                memmagclrlink(mag);
//                bktid2++;
                bktid1++;
                ofs <<= 1;
            }
        }
        if (!mag1) {
            mag1 = mag;
        }
        if (mag1 + ofs < (struct memmag *)pool->hdrtab + nblk) {
            mag2 = mag1 + ofs;
            bktid2 = memmaggetbkt(mag2);
            if (bktid2 == bktid1 && memmagisfree(mag2)) {
                next++;
                ret++;
                mag3 = memmaggetprev(mag2);
                mag4 = memmaggetnext(mag2);
                if ((mag3) && (mag4)) {
                    memmagsetnext(mag3, mag4);
                    memmagsetprev(mag4, mag3);
                } else if (mag3) {
                    memmagsetnext(mag3, mag4); // NULL;
                } else if (mag4) {
                    memmagsetprev(mag4, mag3); // NULL
                    bkt[bktid1].list = mag4;
                } else {
                    bkt[bktid1].list = NULL;
                }
                memmagclrinfo(mag2);
                memmagclrlink(mag2);
                bktid2++;
                memmagclrinfo(mag1);
                memmagclrlink(mag1);
                memmagsetbkt(mag1, bktid2);
                memmagsetfree(mag1);
                bktid1 = bktid2;
                ofs <<= 1;
                mag1 = NULL;
            }
        }
        if (mag1) {
            mag = mag1;
        }
    }
    if (ret) {
        mag1 = bkt[bktid].list;
        memmagclrinfo(mag);
        memmagclrlink(mag);
        memmagsetbkt(mag, bktid1);
        memmagsetfree(mag);
        if (mag1) {
            memmagsetprev(mag1, mag);
            memmagsetnext(mag, mag1);
        }
        bkt[bktid1].list = mag;
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
slabsplit(struct mempool *pool, struct memmag *mag, unsigned long dest)
{
    struct membkt *bkt = pool->tab;
    unsigned long  bktid = memmaggetbkt(mag);
    uint8_t       *ptr = memgetadr(mag, pool);
    struct memmag *mag1;
    struct memmag *mag2;
    size_t         sz = 1UL << bktid;

    ptr += 1UL << bktid;
    while (--bktid >= dest) {
        sz >>= 1;
        ptr -= sz;
        mag1 = memgetmag(ptr, pool);
        mag2 = bkt[bktid].list;
        memmagclrinfo(mag1);
        memmagclrlink(mag1);
        memmagsetbkt(mag1, bktid);
        memmagsetfree(mag1);
        if (mag2) {
            memmagsetprev(mag2, mag1);
            memmagsetnext(mag1, mag2);
        }
        bkt[bktid].list = mag1;
    }
    mag1 = bkt[dest].list;
    memmagclrinfo(mag);
    memmagclrlink(mag);
    memmagsetbkt(mag, dest);
    memmagsetfree(mag);
    if (mag1) {
        memmagsetprev(mag1, mag);
        memmagsetnext(mag, mag1);
    }
    bkt[dest].list = mag;
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif
        
    return;
}

/*
 * allocate mag; split from larger ones if necessary.
 */
void *
slaballoc(struct mempool *pool, unsigned long nb, unsigned long flg)
{
    struct membkt *bkt = pool->tab;
    unsigned long  bktid1 = memcalcbkt(nb);
    unsigned long  bktid2 = bktid1;
    uint8_t       *ptr = NULL;
    struct memmag *mag1;
    struct memmag *mag2;

    mtxlk(&pool->lk);
    mag1 = bkt[bktid1].list;
    if (!mag1) {
        while (!mag1 && ++bktid2 < PTRBITS) {
            mag1 = pool->tab[bktid2].list;
            if (mag1) {
                mag2 = memmaggetnext(mag1);
                if (mag2) {
                    memmagclrprev(mag2);
                }
                pool->tab[bktid2].list = mag2;
                memmagclrlink(mag1);
                slabsplit(pool, mag1, bktid1);
                mag1 = pool->tab[bktid1].list;
            }
        }
    }
    if (mag1) {
        mag2 = memmaggetnext(mag1);
        if (mag2) {
            memmagclrprev(mag2);
        }
        pool->tab[bktid1].list = mag2;
        memmagclrfree(mag1);
        memmagclrlink(mag1);
        memmagsetflg(mag1, flg);
        ptr = memgetadr(mag1, pool);
    }
    mtxunlk(&pool->lk);
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif

    return ptr;
}

/*
 * free slab; combine to form bigger ones if possible.
 */
void
slabfree(struct mempool *pool, void *ptr)
{
    struct membkt *bkt = pool->tab;
    struct memmag *mag1 = memgetmag(ptr, pool);
    unsigned long  bktid = memmaggetbkt(mag1);
    struct memmag *mag2;

    mtxlk(&pool->lk);
#if (!MEMTEST)
    vmfreephys(ptr, 1UL << bktid);
#endif
    memmagsetfree(mag1);
    if (!slabcomb(pool, mag1)) {
        mag2 = bkt[bktid].list;
        memmagclrlink(mag1);
        if (mag2) {
            memmagsetprev(mag2, mag1);
        }
        memmagsetnext(mag1, mag2);
        bkt[bktid].list = mag1;
    }
    mtxunlk(&pool->lk);
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif

    return;
}

