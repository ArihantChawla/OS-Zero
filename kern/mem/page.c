#include <stddef.h>
#if (MEMTEST)
#include <stdio.h>
#endif
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem.h>
#include <kern/mem/page.h>
#include <kern/unit/ia32/vm.h>

extern struct page   vmphystab[NPAGEPHYS];
extern struct pageq  vmphysq;
extern unsigned long vmnphyspages;
unsigned long        npagefree;

void
pageinitzone(struct pageq *zone, uintptr_t base, unsigned long nbphys)
{
    struct page   *pg = vmphystab + (rounduppow2(base, PAGESIZE) >> PAGESIZELOG2);
    unsigned long  n  = (nbphys - rounduppow2(base, PAGESIZE)) >> PAGESIZELOG2;

    pageqlk(zone);
    vmnphyspages = n;
#if (MEMTEST)
    printf("initializing %ld (%lx) pages\n", n, n);
#endif
    while (n--) {
        pagepush(pg, zone);
        pg++;
    }
    pagequnlk(zone);
}

/* TODO: evict pages from LRU if none free / low water */
struct page *
pagezalloc(struct pageq *zone, struct pageq *lru)
{
    struct page *pg;

    pageqlk(zone);
    pagepop(zone, &pg);
    pagequnlk(zone);

    return pg;
}

void
pagezfree(struct pageq *zone, void *adr)
{
    unsigned long  id;
    struct page   *pg;

    pageqlk(zone);
    id = (uintptr_t)adr >> PAGESIZELOG2;
    pg = &vmphystab[id];
    pagepush(pg, zone);
    pagequnlk(zone);

    return;
}

#if 0
void
pageinitdev(unsigned long id, unsigned long npg)
{
    struct swapdev *dev = &_swapdevtab[id];
    unsigned long   nb = npg * sizeof(swapoff_t);
    struct page    *pg;
    struct pageq   *pq = &dev->freeq;

    dev->npg = npg;
    dev->pgmap = kmalloc(nb);
    bzero(dev->pgmap, nb);
    pg = kmalloc(npg * sizeof(struct page));
    dev->pgtab = pg;
    while (npg--) {
        pagepush(pg, pq);
        pg++;
    }

    return;
}

void
swapfree(uintptr_t adr)
{
    struct swapdev *dev = &_swapdevtab[vmdevid(adr)];
    unsigned long   blk = swapblkid(adr);
    struct page    *pg = dev->pgtab + blk;

    pagepush(pg, &dev->freeq);
    dev->pgmap[blk] = 0;

    return;
}

unsigned long
swapalloc(void)
{
    struct swapdev *dev = &_swapdevtab[0];
    struct swapdev *lim = &_swapdevtab[NSWAPDEV];
    unsigned long   ret = 0;
    struct page    *pg;
    long            l;

    l = 0;
    while (dev < lim && (dev->npg)) {
        pagedeq(&dev->freeq, &pg);
        if (pg) {
            swapsetblk(ret, swapblknum(dev, pg));
            swapsetdev(ret, l);

            return ret;
        }
        dev++;
        l++;
    }

    return ret;
}

void
swapwrite(unsigned long blk, unsigned long phys)
{
    ; /* TODO */
}

void
swapout(void)
{
    struct vmpage *pg = NULL;
    struct vmpage *hdrtab = curproc->vmhdrtab;
    unsigned long  spg;

    vmdeqpage(&pg);
    if (pg) {
        spg = swapalloc();
        swapwrite(spg, vmpageadr(pg, hdrtab));
    }

    return;
}

#endif
