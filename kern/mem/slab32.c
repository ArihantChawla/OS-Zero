#include <stddef.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem/slab.h>
#include <kern/unit/ia32/vm.h>

/* physical memory slabs */
struct slabhdr *_physslabtab[PTRBITS] ALIGNED(PAGESIZE);
long            _physlktab[PTRBITS];

extern unsigned long npagefree;

/*
 * zero slab allocator
 *
 * slabs are power-of-two-sizes.
 * slabs are combined to and split from bigger ones on demand; free regions are
 * kept as big as possible.
 */

void
slabinit(struct slabhdr **zone, struct slabhdr *hdrtab,
         unsigned long base, unsigned long nb)
{
    unsigned long   adr = roundup2(base, SLABMIN);
    unsigned long   bkt = PTRBITS - 1;
    unsigned long   ul = 1UL << bkt;
    struct slabhdr *hdr;

    nb -= adr - base;
    nb = rounddown2(nb, SLABMINLOG2);
//    kprintf("%ul kilobytes kernel virtual memory free\n", nb >> 10);
    while ((nb) && bkt >= SLABMINLOG2) {
        if (nb & ul) {
            hdr = &hdrtab[adr >> SLABMINLOG2];
            slabsetbkt(hdr, bkt);
            slabsetfree(hdr);
            slabclrlink(hdr);
            zone[bkt] = hdr;
            nb -= 1UL << bkt;
            adr += 1UL << bkt;
        }
        bkt--;
        ul >>= 1;
    }

    return;
}

/*
 * combine slab with bigger [previous- or next-address] ones if possible.
 */
long
slabcomb(struct slabhdr **zone, struct slabhdr *hdrtab, struct slabhdr *hdr)
{
    unsigned long   bkt1  = slabgetbkt(hdr);
    long            ret   = 0;
    long            prev  = 1;
    long            next  = 1;
    struct slabhdr *hdr1  = NULL;
    struct slabhdr *hdr2;
    struct slabhdr *hdr3;
    struct slabhdr *hdr4;
    unsigned long   ofs;
    unsigned long   bkt2;

    slablk(bkt1);
    slabsetfree(hdr);
    ofs = 1UL << (bkt1 - SLABMINLOG2);
    while ((prev) || (next)) {
        prev = 0;
        next = 0;
        if (hdr - hdrtab >= ofs) {
            hdr1 = hdr - ofs;
            bkt2 = slabgetbkt(hdr1);
            slablk(bkt2);
            if (bkt2 == bkt1 && slabisfree(hdr1)) {
                ret = 1;
                hdr3 = slabgetprev(hdr1, hdrtab);
                hdr4 = slabgetnext(hdr1, hdrtab);
                if (hdr3) {
                    if (hdr4) {
                        slabsetprev(hdr4, hdr3, hdrtab);
                        slabsetnext(hdr3, hdr4, hdrtab);
                    } else {
                        slabclrnext(hdr3);
                    }
                } else {
                    if (hdr4) {
                        slabclrprev(hdr4);
                    }
                    zone[bkt2] = hdr4;
                }
                bkt2++;
                slabsetbkt(hdr1, bkt2);
                bkt1 = bkt2;
            } else {
                prev = 0;
            }
            slabunlk(bkt2);
        }
        if (!prev) {
            hdr1 = hdr;
        }
        if (hdr1 + ofs < hdrtab + SLABNHDR) {
            hdr2 = hdr1 + ofs;
            bkt2 = slabgetbkt(hdr2);
            slablk(bkt2);
            if (bkt2 == bkt1 && slabisfree(hdr2)) {
                ret = 1;
                hdr3 = slabgetprev(hdr2, hdrtab);
                hdr4 = slabgetnext(hdr2, hdrtab);
                if (hdr3) {
                    if (hdr4) {
                        slabsetprev(hdr4, hdr3, hdrtab);
                        slabsetnext(hdr3, hdr4, hdrtab);
                    } else {
                        slabclrnext(hdr3);
                    }
                } else {
                    if (hdr4) {
                        slabclrprev(hdr4);
                    }
                    zone[bkt2] = hdr4;
                }
                bkt2++;
                slabsetbkt(hdr1, bkt2);
                bkt1 = bkt2;
            } else {
                next = 0;
            }
        } 
        hdr = hdr1;
    }
    if (ret) {
        slabsetbkt(hdr1, bkt1);
        slabclrprev(hdr1);
        if (zone[bkt1]) {
            slabsetprev(zone[bkt1], hdr1, hdrtab);
            slabsetnext(hdr1, zone[bkt1], hdrtab);
        } else {
            slabclrnext(hdr1);
        }
        zone[bkt1] = hdr1;
    }
    slabunlk(bkt1);
                
    return ret;
}

/*
 * split slab into smaller ones to satisfy allocation request.
 * split of N to M gives us one free slab in each of M to N-1 and one to
 * allocate in M.
 */    
void
slabsplit(struct slabhdr **zone, struct slabhdr *hdrtab,
          struct slabhdr *hdr, unsigned long dest)
{
    unsigned long   bkt   = slabgetbkt(hdr);
    unsigned long   nb    = 1UL << bkt;
    uint8_t        *ptr   = slabadr(hdr, hdrtab);
    struct slabhdr *hdr1;
    unsigned long   sz;

    hdr1 = slabgetnext(hdr, hdrtab);
    if (hdr1) {
        slabclrprev(hdr1);
    }
    zone[bkt] = hdr1;
    while (--bkt > dest) {
        hdr1 = slabhdr(ptr, hdrtab);
        sz = 1UL << bkt;
        slabsetbkt(hdr1, bkt);
        slabsetfree(hdr1);
        slabclrprev(hdr1);
        slablk(bkt);
        if (zone[bkt]) {
            slabsetprev(zone[bkt], hdr1, hdrtab);
            slabsetnext(hdr1, zone[bkt], hdrtab);
        } else {
            slabclrnext(hdr1);
        }
        zone[bkt] = hdr1;
        slabunlk(bkt);
        nb -= sz;
        ptr += sz;
        bkt--;
    }
    hdr1 = slabhdr(ptr, hdrtab);
    sz = 1UL << bkt;
    slabsetbkt(hdr1, bkt);
    slabsetfree(hdr1);
    slabclrprev(hdr1);
    if (zone[bkt]) {
        slabsetprev(zone[bkt], hdr1, hdrtab);
        slabsetnext(hdr1, zone[bkt], hdrtab);
    } else {
        slabclrnext(hdr1);
    }
    zone[bkt] = hdr1;
    hdr1 = slabhdr(ptr, hdrtab);
    slabsetbkt(hdr1, bkt);
    slabsetfree(hdr1);
    slabclrprev(hdr1);
    slabsetprev(zone[bkt], hdr1, hdrtab);
    slabsetnext(hdr1, zone[bkt], hdrtab);
    zone[bkt] = hdr1;
        
    return;
}

/*
 * allocate slab; split from larger ones if necessary.
 */
void *
slaballoc(struct slabhdr **zone, struct slabhdr *hdrtab,
          unsigned long nb, unsigned long flg)
{
    unsigned long   bkt1  = max(SLABMINLOG2, slabbkt(nb));
    struct slabhdr *hdr1;
    uint8_t        *ptr   = NULL;
    unsigned long   bkt2;
    struct slabhdr *hdr2;

    slablk(bkt1);
    hdr1 = zone[bkt1];
    if (!hdr1) {
        bkt2 = bkt1;
        while (!hdr1 && ++bkt2 < PTRBITS) {
            slablk(bkt2);
            hdr1 = zone[bkt2];
            if (!hdr1) {
                slabunlk(bkt2);
            }
        }
        if (hdr1) {
            slabsplit(zone, hdrtab, hdr1, bkt1);
            hdr1 = zone[bkt1];
            slabunlk(bkt1);
            slabunlk(bkt2);
        }
    }
    if (hdr1) {
        hdr2 = slabgetnext(hdr1, hdrtab);
        if (hdr2) {
            slabclrprev(hdr2);
        }
        zone[bkt1] = hdr2;
        slabclrfree(hdr1);
        slabclrprev(hdr1);
        slabclrnext(hdr1);
        slabsetflg(hdr1, flg);
        ptr = slabadr(hdr1, hdrtab);
    }
    slabunlk(bkt1);
    if (flg & SLABZERO) {
        bzero(ptr, 1UL << bkt1);
    }

    return ptr;
}

/*
 * free slab; combine to form bigger ones if possible.
 */
void
slabfree(struct slabhdr **zone, struct slabhdr *hdrtab, void *ptr)
{
    struct slabhdr *hdr  = slabhdr(ptr, hdrtab);
    unsigned long   bkt = slabgetbkt(hdr);

    vmfreephys(ptr, 1UL << bkt);
    if (!slabcomb(zone, hdrtab, hdr)) {
        bkt = slabgetbkt(hdr);
        slablk(bkt);
        if (zone[bkt]) {
            slabsetprev(zone[bkt], hdr, hdrtab);
            slabsetnext(hdr, zone[bkt], hdrtab);
        } else {
            slabclrnext(hdr);
        }
        zone[bkt] = hdr;
        slabunlk(bkt);
    }

    return;
}

