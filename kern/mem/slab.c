#include <stddef.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem/mem.h>
#include <kern/mem/mag.h>
#include <kern/mem/slab.h>
#include <kern/unit/ia32/vm.h>
#if (MEMTEST)
#include <stdio.h>
#include <string.h>
#define kprintf printf
#define kbzero bzero
#endif

extern unsigned long  npagefree;
extern uint8_t       *magvirtbitmap;
extern struct maghdr *magvirthdrtab;

struct slabhdr       *slabvirttab[PTRBITS] ALIGNED(PAGESIZE);
volatile long         slabvirtlktab[PTRBITS];
struct slabhdr       *slabvirthdrtab;

#if (SLABHUGELK)
static volatile long  slablk;
#endif
static unsigned long  slabnhdr;
unsigned long         slabvirtbase;

/*
 * zero slab allocator
 *
 * slabs are power-of-two-sizes.
 * slabs are combined to and split from bigger ones on demand; free regions are
 * kept as big as possible.
 */

unsigned long
slabinitvirt(unsigned long base, unsigned long nb)
{
    unsigned long adr = base;
    unsigned long sz = (nb & (SLABMIN - 1)) ? roundup2(nb, SLABMIN) : nb;
#if 0
    unsigned long nmag = sz >> MAGMINLOG2;
#endif
    unsigned long nmag = sz >> SLABMINLOG2;
#if (MAGBITMAP)
    unsigned long bsz = 0;
#else
    unsigned long bsz = sz >> (MAGMINLOG2 + 3);
#endif
    unsigned long nslab = sz >> SLABMINLOG2;

    slabnhdr = nslab;
    kbzero((void *)adr, bsz + nmag * sizeof(struct maghdr) + nslab * sizeof(struct slabhdr));
#if (!MAGBITMAP)
    magvirtbitmap = (uint8_t *)adr;
#endif
    magvirthdrtab = (struct maghdr *)(adr + bsz);
    slabvirthdrtab = (struct slabhdr *)(adr + bsz + nmag * sizeof(struct maghdr));
    adr += bsz + nmag * sizeof(struct maghdr) + nslab * sizeof(struct slabhdr);
    if (adr & (SLABMIN - 1)) {
        adr = roundup2(adr, SLABMIN);
    }

    return adr;
}

void
slabinit(unsigned long base, unsigned long nb)
{
    struct slabhdr **zone = slabvirttab;
    unsigned long   adr = (base & (SLABMIN - 1))
        ? roundup2(base, SLABMIN)
        : base;
    unsigned long   bkt = PTRBITS - 1;
    unsigned long   ul = 1UL << bkt;
    struct slabhdr *hdr;

#if (SLABHUGELK)
    mtxlk(&slablk, MEMPID);
#endif
    slabvirtbase = adr = slabinitvirt(adr, nb);
    if (base != adr) {
        nb -= adr - base;
        nb = rounddown2(nb, SLABMINLOG2);
    }
    kprintf("%lu kilobytes kernel virtual memory free @ %lx\n", nb >> 10, adr);
    while ((nb) && bkt >= SLABMINLOG2) {
        if (nb & ul) {
            hdr = slabgethdr(adr, slabvirthdrtab, slabvirtbase);
            slabclrnfo(hdr);
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
#if (SLABHUGELK)
    mtxunlk(&slablk, MEMPID);
#endif

    return;
}

/*
 * combine slab with bigger [previous- or next-address] ones if possible.
 */
unsigned long
slabcomb(struct slabhdr **zone, struct slabhdr *hdrtab, struct slabhdr *hdr)
{
    unsigned long   bkt1 = slabgetbkt(hdr);
    unsigned long   bkt2 = bkt1;
    unsigned long   ret  = 0;
    long            prev = 1;
    long            next = 1;
    unsigned long   ofs = 1UL << (bkt1 - SLABMINLOG2);
    struct slabhdr *hdr1;
    struct slabhdr *hdr2;
    struct slabhdr *hdr3;
    struct slabhdr *hdr4;

    while ((prev) || (next)) {
        prev ^= prev;
        next ^= next;
        if (hdr - (struct slabhdr *)hdrtab >= ofs) {
            hdr1 = hdr - ofs;
            bkt2 = slabgetbkt(hdr1);
            if (bkt2 == bkt1) {
                if (slabisfree(hdr1)) {
                    prev = 1;
                    ret++;
                    hdr3 = slabgetprev(hdr1, hdrtab);
                    hdr4 = slabgetnext(hdr1, hdrtab);
                    if ((hdr3) && (hdr4)) {
                        slabsetnext(hdr3, hdr4, hdrtab);
                        slabsetprev(hdr4, hdr3, hdrtab);
                    } else if (hdr3) {
                        slabsetnext(hdr3, hdr4, hdrtab); // NULL;
                    } else if (hdr4) {
                        slabsetprev(hdr4, hdr3, hdrtab); // NULL
                        zone[bkt1] = hdr4;
                    } else {
                        zone[bkt1] = NULL;
                    }
                    slabclrnfo(hdr);
                    slabsetbkt(hdr, 0);
                    bkt2++;
                    slabclrnfo(hdr1);
                    slabclrlink(hdr1);
                    slabsetbkt(hdr1, bkt2);
                    slabsetfree(hdr1);
                    hdr = hdr1;
                    bkt1 = bkt2;
                    ofs <<= 1;
                }
            }
        }
        hdr1 = hdr;
        if (hdr1 + ofs < (struct slabhdr *)hdrtab + slabnhdr) {
            hdr2 = hdr1 + ofs;
            bkt2 = slabgetbkt(hdr2);
            if (bkt2 == bkt1) {
                if (slabisfree(hdr2)) {
                    next = 1;
                    ret++;
                    hdr3 = slabgetprev(hdr2, hdrtab);
                    hdr4 = slabgetnext(hdr2, hdrtab);
                    if ((hdr3) && (hdr4)) {
                        slabsetnext(hdr3, hdr4, hdrtab);
                        slabsetprev(hdr4, hdr3, hdrtab);
                    } else if (hdr3) {
                        slabsetnext(hdr3, hdr4, hdrtab); // NULL;
                    } else if (hdr4) {
                        slabsetprev(hdr4, hdr3, hdrtab); // NULL
                        zone[bkt1] = hdr4;
                    } else {
                        zone[bkt1] = NULL;
                    }
                    slabclrnfo(hdr2);
                    slabsetbkt(hdr2, 0);
                    bkt2++;
                    slabclrnfo(hdr1);
                    slabclrlink(hdr1);
                    slabsetbkt(hdr1, bkt2);
                    slabsetfree(hdr1);
                    bkt1 = bkt2;
                    ofs <<= 1;
                }
            }
        }
        hdr = hdr1;
    }
    if (ret) {
        slabsetfree(hdr);
        slabclrlink(hdr);
        if (zone[bkt1]) {
            slabsetprev(zone[bkt1], hdr, hdrtab);
            slabsetnext(hdr, zone[bkt1], hdrtab);
        }
        zone[bkt1] = hdr;
    }
                
    return ret;
}

/*
 * split slab into smaller ones to satisfy allocation request.
 * split of N to M gives us one free slab in each of M to N-1 and one to
 * allocate in M.
 * caller has locked the bucket dest.
 */    
void
slabsplit(struct slabhdr **zone, struct slabhdr *hdrtab,
          struct slabhdr *hdr, unsigned long dest)
{
    unsigned long   bkt = slabgetbkt(hdr);
    uint8_t        *ptr = slabgetadr(hdr, hdrtab);
    struct slabhdr *hdr1;
    unsigned long   sz = 1UL << bkt;

    hdr1 = slabgetnext(hdr, hdrtab);
    if (hdr1) {
        slabclrlink(hdr1);
        slabsetnext(hdr1, zone[bkt], hdrtab);
    }
    zone[bkt] = hdr1;
    while (--bkt >= dest) {
        sz >>= 1;
        hdr1 = slabgethdr(ptr, hdrtab, slabvirtbase);
        slabclrnfo(hdr1);
        slabclrlink(hdr1);
        slabsetbkt(hdr1, bkt);
        slabsetfree(hdr1);
        if (bkt != dest) {
            slablkq(slabvirtlktab, bkt);
        }
        if (zone[bkt]) {
            slabsetprev(zone[bkt], hdr1, hdrtab);
        }
        slabsetnext(hdr1, zone[bkt], hdrtab);
        zone[bkt] = hdr1;
        if (bkt != dest) {
            slabunlkq(slabvirtlktab, bkt);
        }
        ptr += sz;
    }
    hdr1 = slabgethdr(ptr, hdrtab, slabvirtbase);
    slabclrnfo(hdr1);
    slabclrlink(hdr1);
    slabsetbkt(hdr1, dest);
    slabsetfree(hdr1);
    if (zone[dest]) {
        slabsetprev(zone[dest], hdr1, hdrtab);
        slabsetnext(hdr1, zone[dest], hdrtab);
    }
    zone[dest] = hdr1;
        
    return;
}

/*
 * allocate slab; split from larger ones if necessary.
 */
void *
slaballoc(struct slabhdr **zone, struct slabhdr *hdrtab,
          unsigned long nb, unsigned long flg)
{
    unsigned long   bkt1 = max(SLABMINLOG2, memgetbkt(nb));
    unsigned long   bkt2 = bkt1;
    uint8_t        *ptr = NULL;
    struct slabhdr *hdr1;
    struct slabhdr *hdr2;

#if (SLABHUGELK)
    mtxlk(&slablk, MEMPID);
#endif
    hdr1 = zone[bkt1];
    if (!hdr1) {
        while (!hdr1 && ++bkt2 < PTRBITS) {
            hdr1 = zone[bkt2];
            if (hdr1) {
                slabsplit(zone, hdrtab, hdr1, bkt1);
                hdr1 = zone[bkt1];
            }
        }
    }
    if (hdr1) {
        hdr2 = slabgetnext(hdr1, hdrtab);
        if (hdr2) {
            slabclrprev(hdr2);
        }
        zone[bkt1] = hdr2;
        slabclrfree(hdr1);
        slabclrlink(hdr1);
        slabsetflg(hdr1, flg);
        ptr = slabgetadr(hdr1, hdrtab);
    }
#if (SLABHUGELK)
    mtxunlk(&slablk, MEMPID);
#endif

    return ptr;
}

/*
 * free slab; combine to form bigger ones if possible.
 */
void
slabfree(struct slabhdr **zone, struct slabhdr *hdrtab, void *ptr)
{
    struct slabhdr *hdr = slabgethdr(ptr, hdrtab, slabvirtbase);
    unsigned long   bkt = slabgetbkt(hdr);

#if (SLABHUGELK)
    mtxlk(&slablk, MEMPID);
#endif
#if (!MEMTEST)
    vmfreephys(ptr, 1UL << bkt);
#endif
    slabsetfree(hdr);
    if (!slabcomb(zone, hdrtab, hdr)) {
        slabclrlink(hdr);
        if (zone[bkt]) {
            slabsetprev(zone[bkt], hdr, hdrtab);
        }
        slabsetnext(hdr, zone[bkt], hdrtab);
        zone[bkt] = hdr;
    }
#if (SLABHUGELK)
    mtxunlk(&slablk, MEMPID);
#endif

    return;
}

