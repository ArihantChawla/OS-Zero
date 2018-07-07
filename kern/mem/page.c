#include <stddef.h>
#if (MEMTEST)
#include <stdio.h>
#endif
#include <limits.h>
#include <sys/types.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <zero/trix.h>
#include <zero/mtx.h>
#include <kern/util.h>
#include <kern/malloc.h>
#include <kern/time.h>
#include <kern/mem/vm.h>
#include <kern/mem/page.h>

#if (VMFLATPHYSTAB)
extern struct vmpage      vmphystab[NPAGEMAX];
#endif
//extern m_atomic_t       vmlrulktab[PTRBITS];
extern struct vmpage      vmlrutab[PTRBITS];
extern struct vmpagestat  vmpagestat;
extern VM_LK_T            vmphyslk;
extern struct vmpage     *vmphysqueue;
extern struct vmpage     *vmshmqueue;
#if 0
static m_atomic_t         vmsetlk;
//pid_t                   vmsetmap[NPAGEMAX];
unsigned char             vmsetbitmap[NPAGEMAX / CHAR_BIT];
#endif

unsigned long
pageinitphyszone(uintptr_t base,
                 struct vmpage **zone,
                 size_t nbphys)
{
    struct vmpage *page = &vmphystab[pagenum(base)];
    uintptr_t      adr = rounduppow2(base, PAGESIZE);
    unsigned long  n = rounddownpow2(nbphys - adr, PAGESIZE) >> PAGESIZELOG2;
    unsigned long  size = n * PAGESIZE;
    unsigned long  end = base - 1;

#if 0
    adr += n << PAGESIZELOG2;
    page += n;
#endif
    end += size;
    vmpagestat.nphys = n;
    vmpagestat.phys = (void *)base;
    vmpagestat.physend = (void *)end;
    while (n--) {
//        page--;
        page->adr = adr;
        page->nmap = 0;
//        deqpush(page, zone);
//        adr -= PAGESIZE;
        deqappend(page, zone);
        adr += PAGESIZE;
        page++;
    }

    return size;
}

unsigned long
pageaddphyszone(uintptr_t base,
                struct vmpage **zone,
                size_t nbphys)
{
    uintptr_t      adr = rounduppow2(base, PAGESIZE);
    struct vmpage *page = &vmphystab[pagenum(adr)];
    uint32_t      *pte = (uint32_t *)&_pagetab + vmpagenum(adr);
    unsigned long  n  = rounduppow2(nbphys - adr, PAGESIZE) >> PAGESIZELOG2;
    unsigned long  size = n * PAGESIZE;

#if 0
    adr += n << PAGESIZELOG2;
    page += n;
#endif
    vmpagestat.nphys += n;
    kprintf("reserving %ld (%lx) maps @ %p (%lx)\n",
            n, n, vmphystab, pagenum(base));
    while (n--) {
        if (!*pte) {
//            page--;
            page->adr = adr;
            page->nmap = 0;
//            deqpush(page, zone);
//            adr -= PAGESIZE;
            deqappend(page, zone);
            adr += PAGESIZE;
            page++;
        }
        pte++;
    }

    return size;
}

unsigned long
pageinitphys(uintptr_t base, size_t nbphys)
{
    unsigned long size;

//    vmspinlk(&vmphyslk);
    size = pageinitphyszone(base, &vmphysqueue, nbphys);
//    vmunlkpage(&vmphyslk);

    return size;
}

/* allocate identity-mapped frame */
void *
pagevalloc(void)
{
    struct vmpage *page;

    page = deqpop(&vmshmqueue);
    if (page) {

        return (void *)page->adr;
    }

    return NULL;
}

/*
 * TODO: evict pages from LRU if none free / low water
 */
struct vmpage *
pageallocphys(void)
{
    struct vmpage  *page = NULL;
    struct vmpage **queue;
    long            found = 0;
    long            qid;
    long            q;

    vmlk(&vmphyslk);
    page = deqpop(&vmphysqueue);
    vmunlk(&vmphyslk);
    if (!page) {
        do {
            for (q = 0 ; q < LONGSIZE * CHAR_BIT ; q++) {
                vmlk(&vmlrutab[q].lk);
                queue = &vmlrutab[q].next;
                page = deqgetlast(queue);
                if (page) {
                    found++;
                    page->nmap++;
                    qid = pagecalcqid(page);
                    if (qid != q) {
                        vmlk(&vmlrutab[q].lk);
                    }
                    queue = &vmlrutab[qid].next;
                    deqpush(page, queue);
                    if (qid != q) {
                        vmunlk(&vmlrutab[qid].lk);
                    }

                    break;
                }
                vmunlk(&vmlrutab[q].lk);
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
    unsigned long  id;
    struct vmpage *page;

    /* free physical page */
    vmlk(&vmphyslk);
    id = vmpagenum(adr);
    page = &vmphystab[id];
    vmlkpage(&page->lk);
    if (!--page->nref) {
        vmflushtlb(adr);
        deqpush(page, &vmphysqueue);
    }
    vmunlkpage(&page->lk);
    vmunlk(&vmphyslk);

    return;
}

#if 0
void
pageinitdev(unsigned long id, unsigned long npage)
{
    struct swapdev *dev = &_swapdevtab[id];
    unsigned long   nbmap = npage * sizeof(swapoff_t);
    unsigned long   nbhdr = npage * sizeof(struct vmpage);
    struct vmpage  *page;
    struct vmpage  *pq = dev->freeq;

    dev->npage = npage;
    dev->pagemap = kmalloc(nbmap);
    kbzero(dev->pagemap, nbmap);
    page = kmalloc(nbhdr);
    dev->pagetab = page;
    while (npage--) {
        deqpush(page, pq);
        page++;
    }

    return;
}

void
swapfree(uintptr_t adr)
{
    struct swapdev *dev = &_swapdevtab[vmdevid(adr)];
    unsigned long   blk = swapblkid(adr);
    struct vmpage  *page = dev->pagetab + blk;

    deqpush(page, &dev->freeq);
    dev->pagemap[blk] = 0;

    return;
}

unsigned long
swapalloc(void)
{
    struct swapdev *dev = &_swapdevtab[0];
    struct swapdev *lim = &_swapdevtab[NSWAPDEV];
    unsigned long   ret = 0;
    struct vmpage  *page;
    long            ndx;

    ndx = 0;
    while (dev < lim && (dev->npage)) {
        page = deqgetlast(&dev->freeq);
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

