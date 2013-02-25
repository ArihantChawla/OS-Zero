#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>

#define __KERNEL__ 1
#include <zero/mtx.h>
#define pageqlk(pq)   mtxlk(&(pq)->lk)
#define pagequnlk(pq) mtxunlk(&(pq)->lk)

void *pagealloc(uintptr_t adr);
void  pagefree(void *adr);
void  swapfree(uintptr_t adr);

#define swapblknum(sp, pg)  ((pg) - (sp)->pgtab)
#define pageadr(pg, pt)                                                 \
    ((!(pg))                                                            \
     ? NULL                                                             \
     : ((void *)(((pg) - (pt)) << PAGESIZELOG2)))

#if 0
struct upage {
//    struct perm *perm;
    struct page *prev;
    struct page *next;
};
#endif

struct page {
    uintptr_t xptr;
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

#define pagepop(pq, rpp)                                                \
    do {                                                                \
        struct page  *_pg1;                                             \
        struct page  *_pg2 = NULL;                                      \
                                                                        \
        _pg1 = (pq)->head;                                              \
        if (_pg1) {                                                     \
            _pg2 = (struct page *)_pg1->xptr;                           \
        }                                                               \
        if (_pg2) {                                                     \
            _pg2->xptr ^= (uintptr_t)_pg1;                              \
        } else {                                                        \
            (pq)->tail = NULL;                                          \
        }                                                               \
        (pq)->head = _pg2;                                              \
        *(rpp) = _pg1;                                                  \
    } while (0)
      
#define pagepush(pg, pq)                                                \
    do {                                                                \
        uintptr_t _pg;                                                  \
                                                                        \
        _pg = (uintptr_t)(pq)->head;                                    \
        if (_pg) {                                                      \
            (pq)->head->xptr ^= (uintptr_t)(pg);                        \
        } else {                                                        \
            (pq)->tail = (pg);                                          \
        }                                                               \
        pg->xptr = _pg;                                                 \
        (pq)->head = pg;                                                \
    } while (0)

#define pagedeq(pq, rpp)                                                \
    do {                                                                \
        struct page *_pg1;                                              \
        struct page *_pg2;                                              \
                                                                        \
        _pg1 = (pq)->tail;                                              \
        if (_pg1 == (pq)->head) {                                       \
            (pq)->head = (pq)->tail = NULL;                             \
        } else {                                                        \
            _pg2 = (struct page *)_pg1->xptr;                           \
            _pg2->xptr ^= (uintptr_t)_pg1;                              \
            (pq)->tail = _pg2;                                          \
        }                                                               \
        *(rpp) = _pg1;                                                  \
    } while (0)

#endif /* __PAGE_H__ */

