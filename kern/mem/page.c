#include <stddef.h>
#if (MEMTEST)
#include <stdio.h>
#endif
#include <limits.h>
#include <sys/types.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/mem.h>
#include <kern/time.h>
#include <kern/mem/page.h>
#include <kern/unit/ia32/vm.h>

extern struct page       vmphystab[NPAGEPHYS];
extern struct pageq      vmlrutab[1UL << (LONGSIZELOG2 + 3)];
extern struct vmpagestat vmpagestat;
extern struct pageq      vmphysq;
extern struct pageq      vmshmq;
static volatile long     vmsetlk;
pid_t                    vmsetmap[NPAGEPHYS];

void
pageinitzone(uintptr_t base,
             struct pageq *zone,
             unsigned long nb)
{
    struct page   *page = &vmphystab[pagenum(base)];
    uintptr_t      adr = rounduppow2(base, PAGESIZE);
    unsigned long  n  = max(1, (nb - adr) >> PAGESIZELOG2);

    adr += n << PAGESIZELOG2;
    page += n;
//    mtxlk(&zone[0]->lk);
    vmpagestat.nphys = n;
    kprintf("initializing %ld (%lx) pages @ %p (%lx)\n",
            n, n, vmphystab, pagenum(base));
    while (n--) {
        page--;
        page->adr = adr;
        page->nflt = 0;
        pagepush(zone, page);
        adr -= PAGESIZE;
    }
//    mtxunlk(&zone[0]->lk);
}

void
pageaddzone(uintptr_t base,
            struct pageq *zone,
            unsigned long nb)
{
    uintptr_t      adr = rounduppow2(base, PAGESIZE);
    struct page   *page = &vmphystab[pagenum(adr)];
    uint32_t      *pte = (uint32_t *)&_pagetab + vmpagenum(adr);
    unsigned long  n  = max(1, (nb - adr) >> PAGESIZELOG2);

    adr += n << PAGESIZELOG2;
    page += n;
//    mtxlk(&zone[0]->lk);
    vmpagestat.nphys = n;
    kprintf("reserving %ld (%lx) maps @ %p (%lx)\n",
            n, n, vmphystab, pagenum(base));
    while (n--) {
        if (!*pte) {
            page--;
            page->adr = adr;
            page->nflt = 0;
            pagepush(zone, page);
            adr -= PAGESIZE;
        }
        pte++;
    }
//    mtxunlk(&zone[0]->lk);
}

void
pageinit(uintptr_t base, unsigned long nb)
{
    pageinitzone(base, &vmphysq, max(nb, DEVMEMBASE));

    return;
}

/* allocate identity-mapped frame */
void *
pagevalloc(void)
{
    struct page *page;

    pagepop(&vmshmq, &page);
    if (page) {

        return (void *)page->adr;
    }

    return NULL;
}

/*
 * TODO: evict pages from LRU if none free / low water
 * - skip pages currently in someone's working set
 */
struct page *
pagealloc(void)
{
    struct page  *page = NULL;
    struct pageq *qp;
    long          qid;
    long          l;

//    mtxlk(&vmphysq.lk);
    pagepop(&vmphysq, &page);
//    mtxunlk(&vmphysq.lk);
    if (!page) {
        ktime_t maptm = kcurtime();
        
        for (l = 0 ; l < LONGSIZE * CHAR_BIT ; l++) {
            qp = &vmlrutab[l];
//            mtxlk(&qp->lk);
            if (pageinset(page) && maptm - page->maptm < 10 * KTIME_SECOND) {

                continue;
            } else if (!pageinset(page)) {
                pagedeq(qp, &page);
//            mtxunlk(&qp->lk);
                if (page) {
                    page->nflt++;
                    
                    return page;
                }
                qid = pagegetqid(page);
                qp = &vmlrutab[qid];
//            mtxlk(&qp->lk);
                pagepush(qp, page);
//            mtxunlk(&qp->lk);
            }
        }
    }
    if (page) {
        page->maptm = kcurtime();
        page->dev = PAGENODEV;
        page->ofs = PAGENOOFS;
    }

    return page;
}

void
pagefree(void *adr)
{
    unsigned long  id;
    struct page   *page;

    vmflushtlb(adr);
    /* free physical page */
//    mtxlk(&vmphysq.lk);
    id = (uintptr_t)adr >> PAGESIZELOG2;
    page = &vmphystab[id];
    pagepush(&vmphysq, page);
//    mtxunlk(&vmphysq.lk);

    return;
}

#if 0
void
pageinitdev(unsigned long id, unsigned long npage)
{
    struct swapdev *dev = &_swapdevtab[id];
    unsigned long   nb = npage * sizeof(swapoff_t);
    struct page    *page;
    struct pageq   *pq = &dev->freeq;

    dev->npage = npage;
    dev->pagemap = kmalloc(nb);
    bzero(dev->pagemap, nb);
    page = kmalloc(npage * sizeof(struct page));
    dev->pagetab = page;
    while (npage--) {
        pagepush(pq, page);
        page++;
    }

    return;
}

void
swapfree(uintptr_t adr)
{
    struct swapdev *dev = &_swapdevtab[vmdevid(adr)];
    unsigned long   blk = swapblkid(adr);
    struct page    *page = dev->pagetab + blk;

    pagepush(&dev->freeq, page);
    dev->pagemap[blk] = 0;

    return;
}

unsigned long
swapalloc(void)
{
    struct swapdev *dev = &_swapdevtab[0];
    struct swapdev *lim = &_swapdevtab[NSWAPDEV];
    unsigned long   ret = 0;
    struct page    *page;
    long            l;

    l = 0;
    while (dev < lim && (dev->npage)) {
        pagedeq(&dev->freeq, &page);
        if (page) {
            swapsetblk(ret, swapblknum(dev, page));
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
    struct vmpage *page = NULL;
    struct vmpage *hdrtab = curproc->vmhdrtab;
    unsigned long  spage;

    vmdeqpage(&page);
    if (page) {
        spage = swapalloc();
        swapwrite(spage, vmpageadr(page, hdrtab));
    }

    return;
}

#endif

