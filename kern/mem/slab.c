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

    fmtxlk(&memphyspoollk);
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
    fmtxunlk(&memphyspoollk);

    return;
}
#endif

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
                    memsetmagnext(mag3, mag4);
                    memsetmagprev(mag4, mag3);
                } else if (mag3) {
                    memsetmagnext(mag3, mag4); // NULL
                } else if (mag4) {
                    memsetmagprev(mag4, mag3); // NULL
                    bkt[bktid1].list = mag4;
                } else {
                    bkt[bktid1].list = NULL;
                }
                memclrmaginfo(mag);
                memclrmaglink(mag);
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
                    memsetmagnext(mag3, mag4);
                    memsetmagprev(mag4, mag3);
                } else if (mag3) {
                    memsetmagnext(mag3, mag4); // NULL;
                } else if (mag4) {
                    memsetmagprev(mag4, mag3); // NULL
                    bkt[bktid1].list = mag4;
                } else {
                    bkt[bktid1].list = NULL;
                }
                memclrmaginfo(mag2);
                memclrmaglink(mag2);
                bktid2++;
                memclrmaginfo(mag1);
                memclrmaglink(mag1);
                memsetmagbkt(mag1, bktid2);
                memsetmagfree(mag1);
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
        memclrmaginfo(mag);
        memclrmaglink(mag);
        memsetmagbkt(mag, bktid1);
        memsetmagfree(mag);
        if (mag1) {
            memsetmagprev(mag1, mag);
            memsetmagnext(mag, mag1);
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
        memclrmaginfo(mag1);
        memclrmaglink(mag1);
        memsetmagbkt(mag1, bktid);
        memsetmagfree(mag1);
        if (mag2) {
            memsetmagprev(mag2, mag1);
            memsetmagnext(mag1, mag2);
        }
        bkt[bktid].list = mag1;
    }
    mag1 = bkt[dest].list;
    memclrmaginfo(mag);
    memclrmaglink(mag);
    memsetmagbkt(mag, dest);
    memsetmagfree(mag);
    if (mag1) {
        memsetmagprev(mag1, mag);
        memsetmagnext(mag, mag1);
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

    fmtxlk(&pool->lk);
    mag1 = bkt[bktid1].list;
    if (!mag1) {
        while (!mag1 && ++bktid2 < PTRBITS) {
            mag1 = pool->tab[bktid2].list;
            if (mag1) {
                mag2 = memmaggetnext(mag1);
                if (mag2) {
                    memclrmagprev(mag2);
                }
                pool->tab[bktid2].list = mag2;
                memclrmaglink(mag1);
                slabsplit(pool, mag1, bktid1);
                mag1 = pool->tab[bktid1].list;
            }
        }
    }
    if (mag1) {
        mag2 = memmaggetnext(mag1);
        if (mag2) {
            memclrmagprev(mag2);
        }
        pool->tab[bktid1].list = mag2;
        memclrmagfree(mag1);
        memclrmaglink(mag1);
        memsetmagflg(mag1, flg);
        ptr = memgetadr(mag1, pool);
    }
    fmtxunlk(&pool->lk);
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

    fmtxlk(&pool->lk);
#if (!MEMTEST)
    vmfreephys(ptr, 1UL << bktid);
#endif
    memsetmagfree(mag1);
    if (!slabcomb(pool, mag1)) {
        mag2 = bkt[bktid].list;
        memclrmaglink(mag1);
        if (mag2) {
            memsetmagprev(mag2, mag1);
        }
        memsetmagnext(mag1, mag2);
        bkt[bktid].list = mag1;
    }
    fmtxunlk(&pool->lk);
#if (__KERNEL__ && (MEMDIAG))
    memdiag(pool);
#endif

    return;
}

