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
#include <kern/malloc.h>
#include <kern/time.h>
#include <kern/mem/vm.h>
#include <kern/mem/page.h>

extern struct page        vmphystab[NPAGEPHYS];
extern volatile long      vmlrulktab[1UL << (LONGSIZELOG2 + 3)];
extern struct page       *vmlrutab[1UL << (LONGSIZELOG2 + 3)];
extern struct vmpagestat  vmpagestat;
volatile long             vmphysqlk;
extern struct page       *vmphysq;
extern struct page       *vmshmq;
static volatile long      vmsetlk;
//pid_t                     vmsetmap[NPAGEPHYS];
unsigned char             vmsetbitmap[NPAGEPHYS / CHAR_BIT];

void
pageinitzone(uintptr_t base,
             struct page **zone,
             unsigned long nb)
{
    struct page   *page = &vmphystab[pagenum(base)];
    uintptr_t      adr = rounduppow2(base, PAGESIZE);
    unsigned long  n  = (nb - adr) >> PAGESIZELOG2;

    adr += n << PAGESIZELOG2;
    page += n;
    vmpagestat.nphys = n;
    kprintf("initializing %ld (%lx) pages @ %p (%lx)\n",
            n, n, vmphystab, pagenum(base));
    while (n--) {
        page--;
        page->adr = adr;
        page->nflt = 0;
        pagepush(page, zone);
        adr -= PAGESIZE;
    }
}

void
pageaddzone(uintptr_t base,
            struct page **zone,
            unsigned long nb)
{
    uintptr_t      adr = rounduppow2(base, PAGESIZE);
    struct page   *page = &vmphystab[pagenum(adr)];
    uint32_t      *pte = (uint32_t *)&_pagetab + vmpagenum(adr);
    unsigned long  n  = (nb - adr) >> PAGESIZELOG2;

    adr += n << PAGESIZELOG2;
    page += n;
    vmpagestat.nphys = n;
    kprintf("reserving %ld (%lx) maps @ %p (%lx)\n",
            n, n, vmphystab, pagenum(base));
    while (n--) {
        if (!*pte) {
            page--;
            page->adr = adr;
            page->nflt = 0;
            pagepush(page, zone);
            adr -= PAGESIZE;
        }
        pte++;
    }
}

void
pageinit(uintptr_t base, unsigned long nb)
{
    mtxlk(&vmphysqlk);
    pageinitzone(base, &vmphysq, min(nb, DEVMEMBASE));
    mtxunlk(&vmphysqlk);

    return;
}

/* allocate identity-mapped frame */
void *
pagevalloc(void)
{
    struct page *page;

    page = pagepop(&vmshmq);
    if (page) {

        return (void *)page->adr;
    }

    return NULL;
}

/*
 * TODO: evict pages from LRU if none free / low water
 */
struct page *
pagealloc(void)
{
    struct page  *page = NULL;
    struct page **queue;
    long          found = 0;
    long          qid;
    long          q;

    mtxlk(&vmphysqlk);
    page = pagepop(&vmphysq);
    mtxunlk(&vmphysqlk);
    if (!page) {
        do {
            for (q = 0 ; q < LONGSIZE * CHAR_BIT ; q++) {
                mtxlk(&vmlrulktab[q]);
                queue = &vmlrutab[q];
                page = pagedequeue(queue);
                if (page) {
                    found++;
                    page->nflt++;
                    qid = pagegetqid(page);
                    if (qid != q) {
                        mtxlk(&vmlrulktab[qid]);
                    }
                    queue = &vmlrutab[qid];
                    pagepush(page, queue);
                    if (qid != q) {
                        mtxunlk(&vmlrulktab[qid]);
                    }
                    
                    break;
                }
                mtxunlk(&vmlrulktab[q]);
            }
            if (found) {
                
                break;
            }
        } while (!found);
    }
    if (page) {
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

    /* free physical page */
    mtxlk(&vmphysqlk);
    id = (uintptr_t)adr >> PAGESIZELOG2;
    page = &vmphystab[id];
    mtxlk(&page->lk);
    if (--page->nref <= 0) {
        page->nref = 0;
        vmflushtlb(adr);
        pagepush(page, &vmphysq);
    }
    mtxunlk(&page->lk);
    mtxunlk(&vmphysqlk);

    return;
}

#if 0
void
pageinitdev(unsigned long id, unsigned long npage)
{
    struct swapdev *dev = &_swapdevtab[id];
    unsigned long   nbmap = npage * sizeof(swapoff_t);
    unsigned long   nbhdr = npage * sizeof(struct page);
    struct page    *page;
    struct page    *pq = dev->freeq;

    dev->npage = npage;
    dev->pagemap = kmalloc(nbmap);
    kbzero(dev->pagemap, nbmap);
    page = kmalloc(nbhdr);
    dev->pagetab = page;
    while (npage--) {
        pagepush(page, pq);
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

    pagepush(page, &dev->freeq);
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
    long            ndx;

    ndx = 0;
    while (dev < lim && (dev->npage)) {
        page = pagedequeue(&dev->freeq);
        if (page) {
            swapsetblk(ret, swapblknum(dev, page));
            swapsetdev(ret, ndx);

            return ret;
        }
        dev++;
        ndx++;
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

