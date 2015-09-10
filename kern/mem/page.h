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
#define pageinset(pg)  (vmsetmap[pagenum((pg)->adr)])
#define pageaddset(pg) (vmsetmap[pagenum((pg)->adr)] = k_curpid)
extern pid_t           vmsetmap[NPAGEPHYS];

#if 0
struct upage {
//    struct perm *perm;
    struct page *prev;
    struct page *next;
};
#endif

#define PAGEWIREBIT 0x00000001
#define PAGEBUFBIT  0x00000002
struct page {
    struct perm    perm;        // page permissions
    long           flg;         // page flags
    uintptr_t      adr;         // page address
    unsigned long  nflt;        // # of page-fault exceptions triggered
    struct page   *prev;        // previous on queue
    struct page   *next;        // next on queue
//    ktime_t        maptm;       // map timestamp
    dev_t          dev;         // paging device
    off_t          ofs;         // paging device offset
};

struct pageq {
    volatile long  lk;
    struct page   *head;
    struct page   *tail;
};

typedef int64_t swapoff_t;

#define PAGEDEVMASK        ((UINT64_C(1) << PAGESIZELOG2) - 1)
#define swapsetblk(u, blk) ((u) |= (blk) << PAGESIZELOG2)
#define swapsetdev(u, dev) ((u) |= (dev))
#define swapblkid(adr)     ((adr) >> PAGESIZELOG2)
#define swapdevid(adr)     ((adr) & PAGEDEVMASK)
struct swapdev {
    swapoff_t     npg;
    swapoff_t    *pgmap;
    struct page  *pgtab;
    struct pageq  freeq;
};

#define LIST_NOLOCK 1
#define LIST_TYPE  struct page
#define LIST_QTYPE struct pageq
#include <zero/list.h>
#define pagegetqid(pg)   (tzerol(pg->nflt))
#define pagepop(pq, rpp) listpop(pq, rpp)
#define pagepush(pq, pg) listpush(pq, pg)
#define pagedeq(pq, rpp) listdequeue(pq, rpp)
#define pagerm(pq, pg)   listrm(pq, pg)

void          pageinitzone(uintptr_t base, struct pageq *zone, unsigned long nb);
void          pageaddzone(uintptr_t base, struct pageq *zone, unsigned long nb);
void          pageinit(uintptr_t base, unsigned long nb);
struct page * pagealloc(void);
void          pagefree(void *adr);
#if 0
void          pagefree(void *adr);
void          swapfree(uintptr_t adr);
#endif

#endif /* __KERN_MEM_PAGE_H__ */

