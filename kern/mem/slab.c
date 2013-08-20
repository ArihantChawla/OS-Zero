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

extern unsigned long   npagefree;
extern uint8_t        *magvirtbitmap;
#if 0
extern struct maghdr  *magvirthdrtab;
#endif
extern struct memzone  magvirtzone;

struct memzone        slabvirtzone ALIGNED(PAGESIZE);
#if 0
struct slabhdr       *slabvirttab[PTRBITS] ALIGNED(PAGESIZE);
volatile long         slabvirtlktab[PTRBITS];
struct slabhdr       *slabvirthdrtab;
#endif

//static unsigned long  slabnhdr;
//unsigned long         slabvirtbase;
#if (NEWLK)
volatile long         slablk;
#endif
/*
 * zero slab allocator
 *
 * slabs are power-of-two-sizes.
 * slabs are combined to and split from bigger ones on demand; free regions are
 * kept as big as possible.
 */

unsigned long
slabinitzone(struct memzone *zone, unsigned long base, unsigned long nb)
{
    unsigned long adr = base;
    unsigned long sz = (nb & (SLABMIN - 1)) ? rounduppow2(nb, SLABMIN) : nb;
#if 0
    unsigned long nmag = sz >> MAGMINLOG2;
#endif
    unsigned long nmag = sz >> SLABMINLOG2;
#if (MAGBITMAP)
    unsigned long bsz = 0;
#else
    unsigned long bsz = sz >> (MAGMINLOG2 + 3);
#endif
    unsigned long nslab = nmag;

    zone->nhdr = nslab;
    kbzero((void *)adr, bsz + nmag * sizeof(struct maghdr) + nslab * sizeof(struct slabhdr));
#if (!MAGBITMAP)
    magvirtbitmap = (uint8_t *)adr;
#endif
    magvirtzone.nhdr = nmag;
    magvirtzone.hdrtab = (void *)(adr + bsz);
    zone->hdrtab = (struct slabhdr *)(adr + bsz + nmag * sizeof(struct maghdr));
    adr += bsz + nmag * sizeof(struct maghdr) + nslab * sizeof(struct slabhdr);
    if (adr & (PAGESIZE - 1)) {
        adr = rounduppow2(adr, PAGESIZE);
    }

    return adr;
}

void
slabinit(struct memzone *virtzone, unsigned long base, unsigned long nbphys)
{
    struct slabhdr **slabtab = (struct slabhdr **)virtzone->tab;
    unsigned long    adr = ((base & (SLABMIN - 1))
                            ? rounduppow2(base, SLABMIN)
                            : base);
    unsigned long    bkt = PTRBITS - 1;
    unsigned long    ul = 1UL << bkt;
    struct slabhdr  *hdr;

    adr = slabinitzone(virtzone, adr, nbphys);
    virtzone->base = adr;
    magvirtzone.base = adr;
    if (base != adr) {
        nbphys -= adr - base;
        nbphys = rounddownpow2(nbphys, PAGESIZELOG2);
    }
    kprintf("%lu kilobytes kernel virtual memory free @ %lx\n", nbphys >> 10, adr);
    while ((nbphys) && bkt >= SLABMINLOG2) {
        if (nbphys & ul) {
            hdr = slabgethdr(adr, virtzone);
            slabclrnfo(hdr);
            slabsetbkt(hdr, bkt);
            slabsetfree(hdr);
            slabclrlink(hdr);
            slabtab[bkt] = hdr;
            nbphys -= 1UL << bkt;
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
unsigned long
slabcomb(struct memzone *zone, struct slabhdr *hdr)
{
    struct slabhdr **slabtab = (struct slabhdr **)zone->tab;
    struct slabhdr  *hdrtab = zone->hdrtab;
    unsigned long    nhdr = zone->nhdr;
    unsigned long    bkt1 = slabgetbkt(hdr);
    unsigned long    bkt2 = bkt1;
    unsigned long    ret  = 0;
    long             prev = 1;
    long             next = 1;
    unsigned long    ofs = 1UL << (bkt1 - SLABMINLOG2);
    struct slabhdr  *hdr1;
    struct slabhdr  *hdr2;
    struct slabhdr  *hdr3;
    struct slabhdr  *hdr4;

    while ((prev) || (next)) {
        prev ^= prev;
        next ^= next;
        if (hdr - (struct slabhdr *)hdrtab >= ofs) {
            hdr1 = hdr - ofs;
            bkt2 = slabgetbkt(hdr1);
            if (bkt2 == bkt1 && slabisfree(hdr1)) {
                prev = 1;
                ret++;
                hdr3 = slabgetprev(hdr1, zone);
                hdr4 = slabgetnext(hdr1, zone);
                if ((hdr3) && (hdr4)) {
                    slabsetnext(hdr3, hdr4, zone);
                    slabsetprev(hdr4, hdr3, zone);
                } else if (hdr3) {
                    slabsetnext(hdr3, hdr4, zone); // NULL
                } else if (hdr4) {
                    slabsetprev(hdr4, hdr3, zone); // NULL
                    slabtab[bkt1] = hdr4;
                } else {
                    slabtab[bkt1] = NULL;
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
        hdr1 = hdr;
        if (hdr1 + ofs < (struct slabhdr *)hdrtab + nhdr) {
            hdr2 = hdr1 + ofs;
            bkt2 = slabgetbkt(hdr2);
            if (bkt2 == bkt1 && slabisfree(hdr2)) {
                next = 1;
                ret++;
                hdr3 = slabgetprev(hdr2, zone);
                hdr4 = slabgetnext(hdr2, zone);
                if ((hdr3) && (hdr4)) {
                    slabsetnext(hdr3, hdr4, zone);
                    slabsetprev(hdr4, hdr3, zone);
                } else if (hdr3) {
                    slabsetnext(hdr3, hdr4, zone); // NULL;
                } else if (hdr4) {
                    slabsetprev(hdr4, hdr3, zone); // NULL
                    slabtab[bkt1] = hdr4;
                } else {
                    slabtab[bkt1] = NULL;
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
        hdr = hdr1;
    }
    if (ret) {
        slabsetfree(hdr);
        slabclrlink(hdr);
        if (slabtab[bkt1]) {
            slabsetprev(slabtab[bkt1], hdr, zone);
            slabsetnext(hdr, slabtab[bkt1], zone);
        }
        slabtab[bkt1] = hdr;
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
slabsplit(struct memzone *zone, struct slabhdr *hdr, unsigned long dest)
{
    struct slabhdr **slabtab = (struct slabhdr **)zone->tab;
    unsigned long    bkt = slabgetbkt(hdr);
    uint8_t         *ptr = slabgetadr(hdr, zone);
    struct slabhdr  *hdr1;
    unsigned long    sz = 1UL << bkt;

    hdr1 = slabgetnext(hdr, zone);
    if (hdr1) {
        slabclrprev(hdr1);
//        slabsetnext(hdr1, slabtab[bkt], zone);
    }
    slabtab[bkt] = hdr1;
    while (--bkt >= dest) {
        sz >>= 1;
        hdr1 = slabgethdr(ptr, zone);
        slabclrnfo(hdr1);
        slabclrlink(hdr1);
        slabsetbkt(hdr1, bkt);
        slabsetfree(hdr1);
#if 0
        if (bkt != dest) {
            slablkq(slabvirtlktab, bkt);
        }
#endif
        if (slabtab[bkt]) {
            slabsetprev(slabtab[bkt], hdr1, zone);
        }
        slabsetnext(hdr1, slabtab[bkt], zone);
        slabtab[bkt] = hdr1;
#if 0
        if (bkt != dest) {
            slabunlkq(slabvirtlktab, bkt);
        }
#endif
        ptr += sz;
    }
    hdr1 = slabgethdr(ptr, zone);
    slabclrnfo(hdr1);
    slabclrlink(hdr1);
    slabsetbkt(hdr1, dest);
    slabsetfree(hdr1);
    if (slabtab[dest]) {
        slabsetprev(slabtab[dest], hdr1, zone);
        slabsetnext(hdr1, slabtab[dest], zone);
    }
    slabtab[dest] = hdr1;
        
    return;
}

/*
 * allocate slab; split from larger ones if necessary.
 */
void *
slaballoc(struct memzone *zone, unsigned long nb, unsigned long flg)
{
    struct slabhdr **slabtab = (struct slabhdr **)zone->tab;
    unsigned long    bkt1 = max(SLABMINLOG2, memgetbkt(nb));
    unsigned long    bkt2 = bkt1;
    uint8_t         *ptr = NULL;
    struct slabhdr  *hdr1;
    struct slabhdr  *hdr2;

#if (NEWLK)
    mtxlk(&slablk);
#endif
    hdr1 = slabtab[bkt1];
    if (!hdr1) {
        while (!hdr1 && ++bkt2 < PTRBITS) {
            hdr1 = slabtab[bkt2];
            if (hdr1) {
                slabsplit(zone, hdr1, bkt1);
                hdr1 = slabtab[bkt1];
            }
        }
    }
    if (hdr1) {
        hdr2 = slabgetnext(hdr1, zone);
        if (hdr2) {
            slabclrprev(hdr2);
        }
        slabtab[bkt1] = hdr2;
        slabclrfree(hdr1);
        slabclrlink(hdr1);
        slabsetflg(hdr1, flg);
        ptr = slabgetadr(hdr1, zone);
    }
#if (NEWLK)
    mtxunlk(&slablk);
#endif

    return ptr;
}

/*
 * free slab; combine to form bigger ones if possible.
 */
void
slabfree(struct memzone *zone, void *ptr)
{
    struct slabhdr **slabtab = (struct slabhdr **)zone->tab;
    struct slabhdr  *hdr = slabgethdr(ptr, zone);
    unsigned long    bkt = slabgetbkt(hdr);

#if (!MEMTEST)
    vmfreephys(ptr, 1UL << bkt);
#endif
#if (NEWLK)
    mtxlk(&slablk);
#endif
    slabsetfree(hdr);
    if (!slabcomb(zone, hdr)) {
        slabclrlink(hdr);
        if (slabtab[bkt]) {
            slabsetprev(slabtab[bkt], hdr, zone);
        }
        slabsetnext(hdr, slabtab[bkt], zone);
        slabtab[bkt] = hdr;
    }
#if (NEWLK)
    mtxunlk(&slablk);
#endif

    return;
}

