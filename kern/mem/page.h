#ifndef __KERN_MEM_PAGE_H__
#define __KERN_MEM_PAGE_H__

#include <stdint.h>

#include <zero/param.h>
#include <zero/trix.h>
#define __KERNEL__ 1
#include <zero/mtx.h>

#include <kern/perm.h>
#include <kern/time.h>
#if defined(__x86_64__) || defined(__amd64__)                           \
    || defined(__i386__) || defined(__i486__)                           \
    || defined(__i586__) || defined(__i686__)
#include <kern/unit/x86/cpu.h>
#endif

#define PAGENODEV (-1)
#define PAGENOOFS (-1)

/* page ID */
#define pagenum(adr)  ((adr) >> PAGESIZELOG2)
//#define swapblknum(sp, pg) ((pg) - (sp)->pgtab)
#if 0
#define pageadr(pg, pt)                                                 \
    ((!(pg)) ? NULL : ((void *)(((pg) - (pt)) << PAGESIZELOG2)))
#endif

/* working sets */
#if 0
#define pageinset(pg)  (vmsetmap[pagenum((pg)->adr)])
#define pageaddset(pg) (vmsetmap[pagenum((pg)->adr)] = k_curpid)
extern pid_t           vmsetmap[NPAGEPHYS];
#endif
#define pageinset(pg)  bitset(vmsetbitmap, pagenum((pg)->adr))
#define pageaddset(pg) setbit(vmsetbitmap, pagenum((pg)->adr))
#define pageclrset(pg) clrbit(vmsetbitmap, pagenum((pg)->adr))

#if 0
#define PAGEWIREBIT 0x00000001
#define PAGEBUFBIT  0x00000002
#endif
struct page {
//    struct perm   *perm;        // page permissions
//    long           flg;         // page flags
    volatile long  lk;
    volatile long  nref;
    uintptr_t      adr;         // page address
    unsigned long  nflt;        // # of page-fault exceptions triggered
    struct page   *prev;        // previous on queue
    struct page   *next;        // next on queue
    dev_t          dev;         // paging device
    off_t          ofs;         // paging device offset
};

typedef int64_t swapoff_t;

#define PAGEDEVMASK        ((UINT64_C(1) << PAGESIZELOG2) - 1)
#define swapsetblk(u, blk) ((u) |= (blk) << PAGESIZELOG2)
#define swapsetdev(u, dev) ((u) |= (dev))
#define swapblkid(adr)     ((adr) >> PAGESIZELOG2)
#define swapdevid(adr)     ((adr) & PAGEDEVMASK)
struct swapdev {
    swapoff_t    npg;
    swapoff_t   *pgmap;
    struct page *pgtab;
    struct page *freeq;
};

#define QUEUE_TYPE struct page
#include <zero/queue.h>
#define pagegetqid(pg)   max(LONGSIZE * CHAR_BIT - 1, lzerol(pg->nflt))
#define pagepop(pq)      queuepop(pq)
#define pagepush(pg, pq) queuepush(pg, pq)
#define pagedequeue(pq)  queuegetlast(pq)
#define pagerm(pg, pq)   queuermitem(pg, pq)

void          pageinitzone(uintptr_t base, struct page **zone,
                           unsigned long nb);
void          pageaddzone(uintptr_t base, struct page **zone,
                          unsigned long nb);
void          pageinit(uintptr_t base, unsigned long nb);
struct page * pagealloc(void);
void          pagefree(void *adr);
#if 0
void          pagefree(void *adr);
void          swapfree(uintptr_t adr);
#endif

#endif /* __KERN_MEM_PAGE_H__ */

