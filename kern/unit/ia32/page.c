#include <stddef.h>
#if (MEMTEST)
#include <stdio.h>
#endif
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem.h>
#include <kern/mem/page.h>
#include <kern/unit/ia32/vm.h>

extern unsigned long vmnphyspages;

struct page    physptab[NPAGEMAX] ALIGNED(PAGESIZE);
struct pageq   physfreeq;
unsigned long  npagefree;

void
pageinit(uintptr_t base, unsigned long nbphys)
{
    struct page   *pg = physptab + (roundup2(base, PAGESIZE) >> PAGESIZELOG2);
    unsigned long  n  = (nbphys - roundup2(base, PAGESIZE)) >> PAGESIZELOG2;

    pageqlk(&physfreeq);
    vmnphyspages = n;
#if (MEMTEST)
    printf("initializing %ld (%lx) pages\n", n, n);
#endif
    while (n--) {
        pagepush(pg, &physfreeq);
        pg++;
    }
    pagequnlk(&physfreeq);
}

void *
pagealloc(uintptr_t virt)
{
    struct page *pg;

    pageqlk(&physfreeq);
    pagepop(&physfreeq, &pg);
    pagequnlk(&physfreeq);

    return pageadr(pg, physptab);
}

void
pagefree(void *adr)
{
    unsigned long  id;
    struct page   *pg;

    pageqlk(&physfreeq);
    id = (uintptr_t)adr >> PAGESIZELOG2;
    pg = &physptab[id];
    pagepush(pg, &physfreeq);
    pagequnlk(&physfreeq);

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
