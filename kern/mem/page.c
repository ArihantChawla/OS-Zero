#include <stdint.h>
#include <stddef.h>
#if (MEMTEST)
#include <stdio.h>
#endif
#include <limits.h>
#include <sys/types.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <zero/trix.h>
#include <mt/mtx.h>
#include <kern/util.h>
#include <kern/malloc.h>
#include <kern/time.h>
#include <kern/mem/vm.h>
#include <kern/mem/page.h>

#if (VMFLATPHYSTAB)
extern struct vmpage      k_vmphystab[NPAGEMAX];
#endif

m_ureg_t
pageinitphyszone(m_ureg_t base,
                 struct vmpage **zone,
                 m_ureg_t nbphys)
{
    struct vmpage *page = &k_vmphystab[pagenum(base)];
    m_ureg_t       adr = rounduppow2(base, PAGESIZE);
    m_ureg_t       n = rounddownpow2(nbphys - adr, PAGESIZE) >> PAGESIZELOG2;
    m_ureg_t       size = n * PAGESIZE;
    m_ureg_t       end = base - 1;

#if 0
    adr += n << PAGESIZELOG2;
    page += n;
#endif
    end += size;
    k_physmem.pagestat.nphys = n;
    k_physmem.pagestat.phys = (void *)base;
    k_physmem.pagestat.physend = (void *)end;
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

m_ureg_t
pageaddphyszone(m_ureg_t base,
                struct vmpage **zone,
                m_ureg_t nbphys)
{
    m_ureg_t      adr = rounduppow2(base, PAGESIZE);
    struct vmpage *page = &k_vmphystab[pagenum(adr)];
    uint32_t      *pte = (uint32_t *)&_pagetab + vmpagenum(adr);
    m_ureg_t       n  = rounduppow2(nbphys - adr, PAGESIZE) >> PAGESIZELOG2;
    m_ureg_t       size = n * PAGESIZE;

#if 0
    adr += n << PAGESIZELOG2;
    page += n;
#endif
    k_physmem.pagestat.nphys += n;
    kprintf("reserving %ld (%lx) maps @ %p (%lx)\n",
            n, n, k_vmphystab, pagenum(base));
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

m_ureg_t
pageinitphys(m_ureg_t base, m_ureg_t nbphys)
{
    m_ureg_t size;

//    vmspinlk(&vmphyslk);
    size = pageinitphyszone(base, &k_physmem.pagequeue, nbphys);
//    vmunlkpage(&vmphyslk);

    return size;
}

/* allocate identity-mapped frame */
void *
pagevalloc(void)
{
    struct vmpage *page;

    page = deqpop(&k_physmem.shmqueue);
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

    vmlk(&k_physmem.lk);
    page = deqpop(&k_physmem.pagequeue);
    vmunlk(&k_physmem.lk);
    if (!page) {
        do {
            for (q = 0 ; q < LONGSIZE * CHAR_BIT ; q++) {
                vmlkpage(&k_physmem.lrutab[q].lk);
                queue = &k_physmem.lrutab[q].next;
                page = deqgetlast(queue);
                if (page) {
                    found++;
                    page->nmap++;
                    qid = pagecalcqid(page);
                    if (qid != q) {
                        vmlkpage(&k_physmem.lrutab[q].lk);
                    }
                    queue = &k_physmem.lrutab[qid].next;
                    deqpush(page, queue);
                    if (qid != q) {
                        vmunlkpage(&k_physmem.lrutab[qid].lk);
                    }

                    break;
                }
                vmunlkpage(&k_physmem.lrutab[q].lk);
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
    m_ureg_t       id;
    struct vmpage *page;

    /* free physical page */
    vmlk(&k_physmem.lk);
    id = vmpagenum(adr);
    page = &k_vmphystab[id];
    vmlkpage(&page->lk);
    if (!--page->nref) {
        vmflushtlb(adr);
        deqpush(page, &k_physmem.pagequeue);
    }
    vmunlkpage(&page->lk);
    vmunlk(&k_physmem.lk);

    return;
}

#if 0
void
pageinitdev(m_ureg_t id, m_ureg_t npage)
{
    struct swapdev *dev = &_swapdevtab[id];
    m_ureg_t        nbmap = npage * sizeof(swapoff_t);
    m_ureg_t        nbhdr = npage * sizeof(struct vmpage);
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
swapfree(m_ureg_t adr)
{
    struct swapdev *dev = &_swapdevtab[vmdevid(adr)];
    m_ureg_t        blk = swapblkid(adr);
    struct vmpage  *page = dev->pagetab + blk;

    deqpush(page, &dev->freeq);
    dev->pagemap[blk] = 0;

    return;
}

m_ureg_t
swapalloc(void)
{
    struct swapdev *dev = &_swapdevtab[0];
    struct swapdev *lim = &_swapdevtab[NSWAPDEV];
    m_ureg_t        ret = 0;
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
swapwrite(m_ureg_t blk, m_ureg_t phys)
{
    ; /* TODO */
}

void
swapout(void)
{
    struct vmpage *page = NULL;
    struct vmpage *hdrtab = curproc->vmhdrtab;
    m_ureg_t       spage;

    vmdeqpage(&page);
    if (page) {
        spage = swapalloc();
        swapwrite(spage, vmpageadr(page, hdrtab));
    }

    return;
}

#endif

