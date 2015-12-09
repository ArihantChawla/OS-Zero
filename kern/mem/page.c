#include <stddef.h>
#if (MEMTEST)
#include <stdio.h>
#endif
#include <limits.h>
#include <sys/types.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <kern/util.h>
#include <kern/malloc.h>
#include <kern/time.h>
#include <kern/mem/vm.h>
#include <kern/mem/page.h>

#if (VMFLATPHYSTAB)
extern struct physpage      vmphystab[NPAGEMAX];
#endif
//extern volatile long      vmlrulktab[PTRBITS];
extern struct physlruqueue  vmlrutab[PTRBITS];
extern struct vmpagestat    vmpagestat;
extern volatile long        vmphyslk;
extern struct physpage     *vmphysqueue;
extern struct physpage     *vmshmqueue;
#if 0
static volatile long        vmsetlk;
//pid_t                     vmsetmap[NPAGEMAX];
unsigned char               vmsetbitmap[NPAGEMAX / CHAR_BIT];
#endif

unsigned long
pageinitphyszone(uintptr_t base,
                 struct physpage **zone,
                 unsigned long nbram)
{
    struct physpage *page = &vmphystab[pagenum(base)];
    uintptr_t        adr = rounduppow2(base, PAGESIZE);
    unsigned long    n = rounddownpow2(nbram - adr, PAGESIZE) >> PAGESIZELOG2;
    unsigned long    size = n * PAGESIZE;
    unsigned long    end = base - 1;

    adr += n << PAGESIZELOG2;
    page += n;
    end += size;
    vmpagestat.nphys = n;
    vmpagestat.phys = (void *)base;
    vmpagestat.physend = (void *)end;
    while (n--) {
        page--;
        page->adr = adr;
        page->nflt = 0;
        queuepush(page, zone);
        adr -= PAGESIZE;
    }

    return size;
}

unsigned long
pageaddphyszone(uintptr_t base,
                struct physpage **zone,
                unsigned long nbram)
{
    uintptr_t        adr = rounduppow2(base, PAGESIZE);
    struct physpage *page = &vmphystab[pagenum(adr)];
    uint32_t        *pte = (uint32_t *)&_pagetab + vmpagenum(adr);
    unsigned long    n  = rounduppow2(nbram - adr, PAGESIZE) >> PAGESIZELOG2;
    unsigned long    size = n * PAGESIZE;

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
            queuepush(page, zone);
            adr -= PAGESIZE;
        }
        pte++;
    }

    return size;
}

unsigned long
pageinitphys(uintptr_t base, unsigned long nbram)
{
    unsigned long size;
    
    mtxlk(&vmphyslk);
    size = pageinitphyszone(base, &vmphysqueue, nbram);
    mtxunlk(&vmphyslk);

    return size;
}

/* allocate identity-mapped frame */
void *
pagevalloc(void)
{
    struct physpage *page;

    page = queuepop(&vmshmqueue);
    if (page) {

        return (void *)page->adr;
    }

    return NULL;
}

/*
 * TODO: evict pages from LRU if none free / low water
 */
struct physpage *
pageallocphys(void)
{
    struct physpage  *page = NULL;
    struct physpage **queue;
    long              found = 0;
    long              qid;
    long              q;

    mtxlk(&vmphyslk);
    page = queuepop(&vmphysqueue);
    mtxunlk(&vmphyslk);
    if (!page) {
        do {
            for (q = 0 ; q < LONGSIZE * CHAR_BIT ; q++) {
                mtxlk(&vmlrutab[q].lk);
                queue = &vmlrutab[q].list;
                page = queuegetlast(queue);
                if (page) {
                    found++;
                    page->nflt++;
                    qid = pagecalcqid(page);
                    if (qid != q) {
                        mtxlk(&vmlrutab[q].lk);
                    }
                    queue = &vmlrutab[qid].list;
                    queuepush(page, queue);
                    if (qid != q) {
                        mtxunlk(&vmlrutab[qid].lk);
                    }
                    
                    break;
                }
                mtxunlk(&vmlrutab[q].lk);
            }
            if (found) {
                
                break;
            }
        } while (!found);
    }

    return page;
}

void
pagefreephys(void *adr)
{
    unsigned long    id;
    struct physpage *page;

    /* free physical page */
    mtxlk(&vmphyslk);
    id = vmpagenum(adr);
    page = &vmphystab[id];
    mtxlk(&page->lk);
    if (!--page->nref) {
        vmflushtlb(adr);
        queuepush(page, &vmphysqueue);
    }
    mtxunlk(&page->lk);
    mtxunlk(&vmphyslk);

    return;
}

#if 0
void
pageinitdev(unsigned long id, unsigned long npage)
{
    struct swapdev  *dev = &_swapdevtab[id];
    unsigned long    nbmap = npage * sizeof(swapoff_t);
    unsigned long    nbhdr = npage * sizeof(struct physpage);
    struct physpage *page;
    struct physpage *pq = dev->freeq;

    dev->npage = npage;
    dev->pagemap = kmalloc(nbmap);
    kbzero(dev->pagemap, nbmap);
    page = kmalloc(nbhdr);
    dev->pagetab = page;
    while (npage--) {
        queuepush(page, pq);
        page++;
    }

    return;
}

void
swapfree(uintptr_t adr)
{
    struct swapdev  *dev = &_swapdevtab[vmdevid(adr)];
    unsigned long    blk = swapblkid(adr);
    struct physpage *page = dev->pagetab + blk;

    queuepush(page, &dev->freeq);
    dev->pagemap[blk] = 0;

    return;
}

unsigned long
swapalloc(void)
{
    struct swapdev  *dev = &_swapdevtab[0];
    struct swapdev  *lim = &_swapdevtab[NSWAPDEV];
    unsigned long    ret = 0;
    struct physpage *page;
    long             ndx;

    ndx = 0;
    while (dev < lim && (dev->npage)) {
        page = queuegetlast(&dev->freeq);
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

