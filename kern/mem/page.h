#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>

#include <zero/trix.h>
#define __KERNEL__ 1
#include <zero/mtx.h>

#define pagenum(adr)       ((adr) >> PAGESIZELOG2)
//#define swapblknum(sp, pg) ((pg) - (sp)->pgtab)
#if 0
#define pageadr(pg, pt)                                                 \
    ((!(pg)) ? NULL : ((void *)(((pg) - (pt)) << PAGESIZELOG2)))
#endif

#if 0
struct upage {
//    struct perm *perm;
    struct page *prev;
    struct page *next;
};
#endif

struct page {
    uintptr_t      adr;
    unsigned long  nflt;
//    uintptr_t     xptr;
    struct page   *prev;
    struct page   *next;
};

struct pageq {
    long         lk;
    struct page *head;
    struct page *tail;
};

typedef int64_t swapoff_t;

#define swapsetblk(u, blk) ((u) |= (blk) << PAGESIZELOG2)
#define swapsetdev(u, dev) ((u) |= (dev) << 3)
#define swapblkid(adr)     ((adr) >> PAGESIZELOG2)
#define swapdevid(adr)     (((adr) & PFDEVMASK) >> 3)
struct swapdev {
    swapoff_t     npg;
    swapoff_t    *pgmap;
    struct page  *pgtab;
    struct pageq  freeq;
};

#define LIST_TYPE  struct page
#define LIST_QTYPE struct pageq
#include <zero/list.h>
#define pagegetqid(pg)   (tzerol(pg->nflt))
#define pagepop(pq, rpp) listpop(pq, rpp)
#define pagepush(pq, pg) listpush(pq, pg)
#define pagedeq(pq, rpp) listdeq(pq, rpp)
#define pagerm(pq, pg)   listrm(pq, pg)

void         pageinitzone(uintptr_t base, struct pageq *zone, unsigned long nb);
void         pageaddzone(uintptr_t base, struct pageq *zone, unsigned long nb);
void         pageinit(uintptr_t base, unsigned long nb);
struct page *pagealloc(void);
void         pagefree(void *adr);
#if 0
void         pagefree(void *adr);
void         swapfree(uintptr_t adr);
#endif

#endif /* __PAGE_H__ */

