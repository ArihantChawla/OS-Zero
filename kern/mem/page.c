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
    struct page   *pg = &vmphystab[pagenum(base)];
    uintptr_t      adr = rounduppow2(base, PAGESIZE);
    unsigned long  n  = max(1, (nb - adr) >> PAGESIZELOG2);

    adr += n << PAGESIZELOG2;
    pg += n;
//    mtxlk(&zone[0]->lk);
    vmpagestat.nphys = n;
    kprintf("initializing %ld (%lx) pages @ %p (%lx)\n",
            n, n, vmphystab, pagenum(base));
    while (n--) {
        pg--;
        pg->adr = adr;
        pg->nflt = 0;
        pagepush(zone, pg);
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
    struct page   *pg = &vmphystab[pagenum(adr)];
    uint32_t      *pte = (uint32_t *)&_pagetab + vmpagenum(adr);
    unsigned long  n  = max(1, (nb - adr) >> PAGESIZELOG2);

    adr += n << PAGESIZELOG2;
    pg += n;
//    mtxlk(&zone[0]->lk);
    vmpagestat.nphys = n;
    kprintf("reserving %ld (%lx) maps @ %p (%lx)\n",
            n, n, vmphystab, pagenum(base));
    while (n--) {
        if (!*pte) {
            pg--;
            pg->adr = adr;
            pg->nflt = 0;
            pagepush(zone, pg);
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
    struct page *pg;

    pagepop(&vmshmq, &pg);
    if (pg) {

        return (void *)pg->adr;
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
    struct page  *pg = NULL;
    struct pageq *qp;
    long          qid;
    long          l;

//    mtxlk(&vmphysq.lk);
    pagepop(&vmphysq, &pg);
//    mtxunlk(&vmphysq.lk);
    if (!pg) {
        for (l = 0 ; l < LONGSIZE * CHAR_BIT ; l++) {
            qp = &vmlrutab[l];
//            mtxlk(&qp->lk);
            if (!pageinset(pg)) {
                pagedeq(qp, &pg);
//            mtxunlk(&qp->lk);
                if (pg) {
                    pg->nflt++;
                    
                    return pg;
                }
                qid = pagegetqid(pg);
                qp = &vmlrutab[qid];
//            mtxlk(&qp->lk);
                pagepush(qp, pg);
//            mtxunlk(&qp->lk);
            }
        }
    }

    return pg;
}

void
pagefree(void *adr)
{
    unsigned long  id;
    struct page   *pg;

    vmflushtlb(adr);
    /* free physical page */
//    mtxlk(&vmphysq.lk);
    id = (uintptr_t)adr >> PAGESIZELOG2;
    pg = &vmphystab[id];
    pagepush(&vmphysq, pg);
//    mtxunlk(&vmphysq.lk);

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
        pagepush(pq, pg);
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

    pagepush(&dev->freeq, pg);
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

