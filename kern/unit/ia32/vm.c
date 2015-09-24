/*
 * X86 page tables are accessed as a flat 4-megabyte set of page table entries.
 * The table is mapped to the address PAGETAB declared in vm.h.
 */

#define PAGEDEV 0

#include <kern/conf.h>
#include <stddef.h>
#include <stdint.h>
#include <zero/trix.h>
#include <zero/cdecl.h>
#include <zero/param.h>
//#include <kern/io/dev.h>
#include <kern/util.h>
//#include <kern/proc/task.h>
#include <kern/mem/page.h>
// #include <kern/io/buf.h>
#include <kern/io/drv/pc/dma.h>
#if (SMP)
#include <kern/unit/ia32/mp.h>
#endif
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>

#define HICORE (1024 * 1024)

extern void pginit(void);

extern uint32_t       kernpagedir[NPTE];
struct page           vmphystab[NPAGEPHYS] ALIGNED(PAGESIZE);
volatile long         vmlrulktab[1UL << (LONGSIZELOG2 + 3)];
struct page          *vmlrutab[1UL << (LONGSIZELOG2 + 3)];

//static struct vmpage  vmpagetab[NPAGEMAX] ALIGNED(PAGESIZE);
#if (PAGEDEV)
static struct dev     vmdevtab[NPAGEDEV];
static volatile long  vmdevlktab[NPAGEDEV];
#endif
struct page          *vmphysq;
struct page          *vmshmq;
struct vmpagestat     vmpagestat;

/*
 * 32-bit page directory is flat 4-megabyte table of page-tables.
 * for virtual address virt,
 *
 *     vmphysadr = pagetab[vmpagenum(virt)]; // physical address
 */
void
vmmapseg(uint32_t *pagetab, uint32_t virt, uint32_t phys, uint32_t lim,
         uint32_t flg)
{
    uint32_t *pte;
    long      n;

    n = rounduppow2(lim - virt, PAGESIZE) >> PAGESIZELOG2;
    pte = pagetab + vmpagenum(virt);
    while (n--) {
        *pte = phys | flg;
        phys += PAGESIZE;
        pte++;
    }

    return;
}

/*
 * initialise virtual memory
 * - no locking; call before going multiprocessor
 * - zero page tables
 * - initialize page directory
 * - map page directory index page
 * - map our segments
 * - initialise paging
 */
void
vminit(void *pagetab)
{
    uint32_t *pde;
    uint32_t  adr;
    long      n;

    /* PHYSICAL MEMORY */

    /* initialize page directory index page */
    pde = kernpagedir;
    adr = (uint32_t)pagetab;
    n = NPDE;
    while (n--) {
        *pde = adr | PAGEPRES | PAGEWRITE;
        adr += NPTE * sizeof(uint32_t);
        pde++;
    }

    /* map page directory index page */
    pde = pagetab + vmpagenum(kernpagedir);
    adr = (uint32_t)&kernpagedir;
    *pde = adr | PAGEPRES | PAGEWRITE;

    /* zero page tables */
    kbzero(pagetab, PAGETABSIZE);

    /* identity-map 0..1M */
    vmmapseg(pagetab, 0, 0,
             HICORE,
             PAGEPRES | PAGEWRITE | PAGENOCACHE | PAGEWIRED);
#if 0
    /* map interrupt vector read-only */
    vmmapseg(pagetab, 0, 0,
             PAGESIZE,
             PAGEPRES | PAGENOCACHE | PAGEWIRED);
    /* map the rest of low 1M read-write */
    vmmapseg(pagetab, PAGESIZE, PAGESIZE,
             HICORE,
             PAGEPRES | PAGEWRITE | PAGENOCACHE | PAGEWIRED);
#endif

#if (SMP)
    vmmapseg(pagetab, (uint32_t)MPENTRY, (uint32_t)MPENTRY,
             (uint32_t)&mpend - (uint32_t)&mpentry + MPENTRY,
             PAGEPRES);
#endif

    /* identity-map kernel low-half boot segment */
    vmmapseg(pagetab, HICORE, HICORE,
             (uint32_t)&_eboot,
             PAGEPRES | PAGEWRITE);

    /* identity-map kernel DMA buffers */
    vmmapseg(pagetab, (uint32_t)&_dmabuf, DMABUFBASE,
             (uint32_t)&_dmabuf + DMABUFSIZE,
             PAGEPRES | PAGEWRITE | PAGENOCACHE | PAGEWIRED);

    /* identity-map page tables */
    vmmapseg(pagetab, (uint32_t)pagetab, (uint32_t)pagetab,
             (uint32_t)pagetab + PAGETABSIZE,
             PAGEPRES | PAGEWRITE | PAGEWIRED);

    /* identity map free RAM */
    vmmapseg(pagetab, (uint32_t)&_epagetab, (uint32_t)&_epagetab,
             KERNVIRTBASE,
             PAGEWRITE);
//    kbzero(&_epagetab, lim - (uint32_t)&_epagetab);

    /* VIRTUAL MEMORY */
    
    /* map kernel text/read-only segments */
    vmmapseg(pagetab, (uint32_t)&_text, vmlinkadr((uint32_t)&_textvirt),
             (uint32_t)&_etextvirt,
             PAGEPRES);

    /* map kernel DATA and BSS segments */
    vmmapseg(pagetab, (uint32_t)&_data, vmlinkadr((uint32_t)&_datavirt),
           (uint32_t)&_ebssvirt,
           PAGEPRES | PAGEWRITE);

    /* identity-map 3.5G..4G */
//    devmap(pagetab, DEVMEMBASE, 512 * 1024 * 1024);

    /* initialize paging */
    pginit();
    
    return;
}

void
vminitphys(uintptr_t base, unsigned long nbphys)
{
    unsigned long nb = min(nbphys, KERNVIRTBASE);
    
    /* initialise physical memory manager */
    pageinit(base, nb);

    return;
}

void *
vmmapvirt(uint32_t *pagetab, void *virt, uint32_t size, uint32_t flg)
{
    uint32_t  adr;
    uint32_t *pte;
    long      n;

    adr = (uint32_t)virt & PFLTPAGEMASK;
    n = rounduppow2(size, PAGESIZE) >> PAGESIZELOG2;
    pte = pagetab + vmpagenum(virt);
    while (n--) {
        *pte = PAGEWRITE | flg;
        pte++;
    }
    
    return (void *)adr;
}

void
vmfreephys(void *virt, uint32_t size)
{
//    struct vmbuf *buf;
    uint32_t  adr;
    uint32_t *pte;
    long      n;
//    long          nref;
//    struct page  *pg;

    n = rounduppow2(size, PAGESIZE) >> PAGESIZELOG2;
    pte = (uint32_t *)((uint8_t *)&_pagetab + vmpagenum(virt));
    while (n--) {
        adr = *pte;
        adr &= PFLTPAGEMASK;
        if (!adr) {

            continue;
        }
        
        if (*pte & PAGESWAPPED) {
//            swapfree(adr);
        } else if (!(*pte & PAGEWIRED)) {
#if 0
            pg = pagefind(adr);
            pagerm(pg);
#endif
            vmpagestat.nmapped++;
        } else {
//                kprintf("UNWIRE\n");
            vmpagestat.nwired++;
        }
        pagefree((void *)adr);
        *pte = 0;
        pte++;
    }

    return;
}

#if (FASTINTR)
FASTCALL
#endif
void
vmpagefault(unsigned long pid, uint32_t adr, uint32_t flags)
{
    uint32_t    *pte = (uint32_t *)&_pagetab + vmpagenum(adr);
    uint32_t     flg = *pte & (PFLTFLGMASK | PAGESYSFLAGS);
    uint32_t     page = *pte;
    struct page *pg = NULL;
//    unsigned long  qid;

    if (!(page & ~(PFLTFLGMASK | PAGESYSFLAGS))) {
        pg = pagealloc();
        if (pg) {
            mtxlk(&pg->lk);
            pg->nref++;
            if (flg & PAGEWIRED) {
                vmpagestat.nwired++;
            } else {
                vmpagestat.nmapped++;
                pg->nflt = 1;
                if (!(page & PAGEWIRED)) {
                    mtxlk(&vmlrulktab[0]);
                    pagepush(pg, &vmlrutab[0]);
                    mtxunlk(&vmlrulktab[0]);
                }
            }
            mtxunlk(&pg->lk);
            *pte = page | flg | PAGEPRES;
        }
#if (PAGEDEV)
    } else if (!(page & PAGEPRES)) {
        // pageout();
        pg = vmpagein(page);
        if (pg) {
            mtxlk(&pg->lk);
            pg->nflt++;
            qid = pagegetqid(pg);
            mtxlk(&vmlrulktab[qid]);
            pagepush(pg, &vmlrutab[qid]);
            mtxunlk(&vmlrulktab[qid]);
            mtxunlk(&pg->lk);
        }
#endif
    }
#if 0
    if (pg) {
        pageaddset(pg);
    }
#endif

    return;
}

#if 0
void
vmseekdev(uint32_t dev, uint64_t ofs)
{
    devseek(vmdevtab[dev], ofs, SEEK_SET);
}

uint32_t
vmpagein(uint32_t adr)
{
    uint32_t     pgid = vmpagenum(adr);
    uint32_t     blk = vmblkid(pgid);
    struct page *pg = pagefind(adr);
    void        *data;

    mtxlk(&vmdevlktab[dev], MEMPID);
    vmseekdev(dev, blk * PAGESIZE);
    pg->nflt++;
//    data = pageread(dev, PAGESIZE);
    mtxunlk(&vmdevlktab[pagedev], MEMPID);
}

void
vmpagefree(uint32_t adr)
{
    ;
}
#endif /* 0 */

