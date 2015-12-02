#include <stddef.h>
#include <stdint.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem/mem.h>
#include <kern/mem/mag.h>
#include <kern/mem/slab.h>
#include <kern/mem/vm.h>
#if (MEMTEST)
#include <stdio.h>
#include <string.h>
#if (__KERNEL__)
#define kprintf printf
#endif
#define kbzero bzero
#endif

#define MEMDIAG   0

extern unsigned long  npagefree;
extern struct memzone magvirtzone;

struct memzone        slabvirtzone ALIGNED(PAGESIZE);
static volatile long  slablk;
/*
 * zero slab allocator
 * -------------------
 * - slabs are power-of-two-sizes
 * - slabs are combined to and split from bigger ones on demand;
 *   free regions are kept as big as possible.
 */

#if !(__KERNEL__) && (MEMDIAG)
void
slabdiag(struct memzone *zone)
{
    long            bkt;
    struct slabhdr *hdr1;
    struct slabhdr *hdr2;
    struct slabhdr *null = NULL;

    mtxlk(&slablk);
    for (bkt = 0 ; bkt < PTRBITS ; bkt++) {
            hdr1 = (struct slabhdr **)zone->tab[bkt];
            if (hdr1) {
                if (slabgetprev(hdr1, zone)) {
#if (__KERNEL__)
                    fprintf(stderr, "DIAG: non-NULL prev on head of list %ld\n",
                            bkt);
#endif
                    slabprint(hdr1);
                    
                    *null = *hdr1;
                }
                if (slabgetbkt(hdr1) != bkt) {
#if (__KERNEL__)
                    fprintf(stderr, "DIAG: invalid bucket ID on head of list %ld\n",
                            bkt);
#endif
                    slabprint(hdr1);
                    
                    
                    *null = *hdr1;
                }
                hdr2 = slabgetnext(hdr1, zone);
                while (hdr2) {
                    if (slabgetprev(hdr2, zone) != hdr1) {
#if (__KERNEL__)
                        fprintf(stderr, "DIAG: invalid prev on list %ld\n",
                                bkt);
#endif
                        slabprint(hdr1);
                        
                        
                        *null = *hdr1;
                    }
                    if (slabgetbkt(hdr2) != bkt) {
#if (__KERNEL__)
                        fprintf(stderr, "DIAG: invalid bucket ID on list %ld\n",
                                bkt);
#endif
                        slabprint(hdr1);
                        
                        
                        *null = *hdr1;
                    }
                    hdr1 = hdr2;
                    hdr2 = slabgetnext(hdr2, zone);
                }
            }
        }
    }
    mtxunlk(&slablk);

    return;
}
#endif

unsigned long
slabinitzone(struct memzone *zone, unsigned long base, unsigned long nb)
{
    unsigned long adr = base;
//    unsigned long sz = (nb & (SLABMIN - 1)) ? rounddownpow2(nb, SLABMIN) : nb;
    unsigned long sz = nb;
    unsigned long ofs = base & (SLABMIN - 1);
    unsigned long nslab;
    unsigned long hdrsz;

    if (ofs) {
        adr += SLABMIN - ofs;
        sz -= adr - base;
    }
    nslab = sz >> SLABMINLOG2;
    /* configure slab headers */
    hdrsz = nslab * sizeof(struct slabhdr);
    hdrsz = rounduppow2(hdrsz, PAGESIZE);
    zone->nhdr = nslab;
//    zone->hdrtab = (void *)adr;
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE);
    zone->hdrtab = (void *)adr;
//    kbzero((void *)adr, hdrsz);
    /* configure magazine headers */
    hdrsz = nslab * sizeof(struct maghdr);
    hdrsz = rounduppow2(hdrsz, PAGESIZE);
    adr += hdrsz;
#if (__KERNEL__)
//    kprintf("MAG: reserved %lu bytes for %lu magazine headers\n", hdrsz, nslab);
#endif
    magvirtzone.nhdr = nslab;
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + hdrsz,
             PAGEPRES | PAGEWRITE);
    magvirtzone.hdrtab = (void *)adr;
//    kbzero((void *)adr, hdrsz);
    adr += hdrsz;
#if (__KERNEL__ && (MEMDIAG))
    slabdiag(zone);
#endif

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
    unsigned long    sz = 1UL << bkt;
    struct slabhdr  *hdr;

    adr = slabinitzone(virtzone, adr, nbphys);
    virtzone->base = adr;
    magvirtzone.base = adr;
    nbphys -= adr - base;
    nbphys = rounddownpow2(nbphys, SLABMIN);
    vmmapseg((uint32_t *)&_pagetab, adr, adr, adr + nbphys,
             PAGEWRITE);
#if (__KERNEL__) && 0
    kprintf("%ld kilobytes kernel virtual memory free @ 0x%lx\n",
            nbphys >> 10, adr);
#endif
    while ((nbphys) && bkt >= SLABMINLOG2) {
        if (nbphys & sz) {
            hdr = slabgethdr(adr, virtzone);
            slabclrinfo(hdr);
            slabclrlink(hdr);
            slabsetbkt(hdr, bkt);
            slabsetfree(hdr);
            slabtab[bkt] = hdr;
            nbphys &= ~sz;
            adr += sz;
        }
        bkt--;
        sz >>= 1;
    }
#if (__KERNEL__ && (MEMDIAG))
    slabdiag(virtzone);
#endif

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
    unsigned long    bkt = slabgetbkt(hdr);
    unsigned long    bkt1 = bkt;
    unsigned long    bkt2 = bkt1;
    unsigned long    ret  = 0;
    long             prev = 1;
    long             next = 1;
    long             ofs = 1UL << (bkt1 - SLABMINLOG2);
    struct slabhdr  *hdr1;
    struct slabhdr  *hdr2;
    struct slabhdr  *hdr3;
    struct slabhdr  *hdr4;

    hdr1 = hdr;
    while ((prev) || (next)) {
        prev ^= prev;
        next ^= next;
        if (hdr - (struct slabhdr *)hdrtab >= ofs) {
            hdr1 = hdr - ofs;
            bkt2 = slabgetbkt(hdr1);
            if (bkt2 == bkt1 && slabisfree(hdr1)) {
                prev++;
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
                slabclrinfo(hdr);
                slabclrlink(hdr);
//                bkt2++;
                bkt1++;
                ofs <<= 1;
            }
        }
        if (!hdr1) {
            hdr1 = hdr;
        }
        if (hdr1 + ofs < (struct slabhdr *)hdrtab + nhdr) {
            hdr2 = hdr1 + ofs;
            bkt2 = slabgetbkt(hdr2);
            if (bkt2 == bkt1 && slabisfree(hdr2)) {
                next++;
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
                slabclrinfo(hdr2);
                slabclrlink(hdr2);
                bkt2++;
                slabclrinfo(hdr1);
                slabclrlink(hdr1);
                slabsetbkt(hdr1, bkt2);
                slabsetfree(hdr1);
                bkt1 = bkt2;
                ofs <<= 1;
                hdr1 = NULL;
            }
        }
        if (hdr1) {
            hdr = hdr1;
        }
    }
    if (ret) {
        slabclrinfo(hdr);
        slabclrlink(hdr);
        slabsetbkt(hdr, bkt1);
        slabsetfree(hdr);
        if (slabtab[bkt1]) {
            slabsetprev(slabtab[bkt1], hdr, zone);
            slabsetnext(hdr, slabtab[bkt1], zone);
        }
        slabtab[bkt1] = hdr;
    }
#if (__KERNEL__ && (MEMDIAG))
    slabdiag(zone);
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
slabsplit(struct memzone *zone, struct slabhdr *hdr, unsigned long dest)
{
    struct slabhdr **slabtab = (struct slabhdr **)zone->tab;
    unsigned long    bkt = slabgetbkt(hdr);
    uint8_t         *ptr = slabgetadr(hdr, zone);
    struct slabhdr  *hdr1;
    unsigned long    sz = 1UL << bkt;

    ptr += 1UL << dest;
    while (--bkt >= dest) {
        sz >>= 1;
        ptr -= sz;
        hdr1 = slabgethdr(ptr, zone);
        slabclrinfo(hdr1);
        slabclrlink(hdr1);
        slabsetbkt(hdr1, bkt);
        slabsetfree(hdr1);
        if (slabtab[bkt]) {
            slabsetprev(slabtab[bkt], hdr1, zone);
            slabsetnext(hdr1, slabtab[bkt], zone);
        }
        slabtab[bkt] = hdr1;
    }
//    hdr1 = slabgethdr(ptr, zone);
    slabclrinfo(hdr);
    slabclrlink(hdr);
    slabsetbkt(hdr, dest);
    slabsetfree(hdr);
    if (slabtab[dest]) {
        slabsetprev(slabtab[dest], hdr, zone);
        slabsetnext(hdr, slabtab[dest], zone);
    }
    slabtab[dest] = hdr;
#if (__KERNEL__ && (MEMDIAG))
    slabdiag(zone);
#endif
        
    return;
}

/*
 * allocate slab; split from larger ones if necessary.
 */
void *
slaballoc(struct memzone *zone, unsigned long nb, unsigned long flg)
{
    struct slabhdr **slabtab = (struct slabhdr **)zone->tab;
    unsigned long    bkt1 = memgetbkt(nb);
    unsigned long    bkt2 = bkt1;
    uint8_t         *ptr = NULL;
    struct slabhdr  *hdr1;
    struct slabhdr  *hdr2;

    mtxlk(&slablk);
    hdr1 = slabtab[bkt1];
    if (!hdr1) {
        while (!hdr1 && ++bkt2 < PTRBITS) {
            hdr1 = slabtab[bkt2];
            if (hdr1) {
                hdr2 = slabgetnext(hdr1, zone);
                if (hdr2) {
                    slabclrprev(hdr2);
                }
                slabtab[bkt2] = hdr2;
                slabclrlink(hdr1);
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
    mtxunlk(&slablk);
#if (__KERNEL__ && (MEMDIAG))
    slabdiag(zone);
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
    struct slabhdr  *hdr1 = slabgethdr(ptr, zone);
    unsigned long    bkt = slabgetbkt(hdr1);

    mtxlk(&slablk);
#if (!MEMTEST)
    vmfreephys(ptr, 1UL << bkt);
#endif
    slabsetfree(hdr1);
    if (!slabcomb(zone, hdr1)) {
        slabclrlink(hdr1);
        if (slabtab[bkt]) {
            slabsetprev(slabtab[bkt], hdr1, zone);
        }
        slabsetnext(hdr1, slabtab[bkt], zone);
        slabtab[bkt] = hdr1;
    }
    mtxunlk(&slablk);
#if (__KERNEL__ && (MEMDIAG))
    slabdiag(zone);
#endif

    return;
}

