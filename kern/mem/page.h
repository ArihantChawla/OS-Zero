#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>

#include <zero/trix.h>
#define __KERNEL__ 1
#include <zero/mtx.h>
#define pageqlk(pq)   mtxlk(&(pq)->lk)
#define pagequnlk(pq) mtxunlk(&(pq)->lk)

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

#define pagegetq(pq) (tzerol(pq->nflt))
#define pagepop(pq, rpp)                                                \
    do {                                                                \
        struct page  *_pg1;                                             \
        struct page  *_pg2 = NULL;                                      \
                                                                        \
        _pg1 = (pq)->head;                                              \
        if (_pg1) {                                                     \
            _pg2 = (struct page *)_pg1->next;                           \
        }                                                               \
        if (_pg2) {                                                     \
            _pg2->prev = NULL;                                          \
        } else {                                                        \
            (pq)->tail = NULL;                                          \
        }                                                               \
        (pq)->head = _pg2;                                              \
        *(rpp) = _pg1;                                                  \
    } while (0)
      
#define pagepush(pg, pq)                                                \
    do {                                                                \
        struct page *_pg;                                               \
                                                                        \
        (pg)->prev = NULL;                                              \
        _pg = (pq)->head;                                               \
        if (_pg) {                                                      \
            (_pg)->prev = (pg);                                         \
        } else {                                                        \
            (pq)->head = (pg);                                          \
            (pq)->tail = (pg);                                          \
        }                                                               \
        pg->next = _pg;                                                 \
        (pq)->head = pg;                                                \
    } while (0)

#define pagedeq(pq, rpp)                                                \
    do {                                                                \
        struct page *_pg1;                                              \
        struct page *_pg2;                                              \
                                                                        \
        _pg1 = (pq)->tail;                                              \
        if (_pg1) {                                                     \
            _pg2 = _pg1->prev;                                          \
            if (!_pg2) {                                                \
                (pq)->head = (pq)->tail = NULL;                         \
            } else {                                                    \
                _pg2->next = NULL;                                      \
                (pq)->tail = _pg2;                                      \
            }                                                           \
            *(rpp) = _pg1;                                              \
        }                                                               \
    } while (0)

#define pagerm(pg)                                                      \
    do {                                                                \
        struct pageq *_pageq;                                           \
        struct page  *_tmp;                                             \
                                                                        \
        pageq = pagegetq(pg);                                           \
        vmlklruq(_pageq);                                               \
        _tmp = (pg)->prev;                                              \
        if (_tmp) {                                                     \
            _tmp->next = (pg)->next;                                    \
        } else {                                                        \
            _tmp = (pg)->next;                                          \
            _pageq->head = _tmp;                                        \
            if (_tmp) {                                                 \
                _tmp->prev = (pg)->prev;                                \
            } else {                                                    \
                _pageq->tail = _tmp;                                    \
            }                                                           \
            _pageq->head = _tmp;                                        \
        }                                                               \
        _tmp = (pg)->next;                                              \
        if (_tmp) {                                                     \
            _tmp->prev = (pg)->prev;                                    \
        } else {                                                        \
            _tmp = (pg)->prev;                                          \
            _pageq->tail = _tmp;                                        \
            if (_tmp) {                                                 \
                _tmp->next = NULL;                                      \
            } else {                                                    \
                _pageq->head = _pageq->tail = _tmp;                     \
            }                                                           \
        }                                                               \
        vmunlklruq(_pageq);                                             \
    } while (0)

void         pageinitzone(uintptr_t base, struct pageq *zone, uintptr_t ofs,
                          unsigned long nb);
struct page *pagezalloc(struct pageq *zone, struct pageq *lru);
void         pagezfree(struct pageq *zone, void *adr);
#if 0
void         pagefree(void *adr);
void         swapfree(uintptr_t adr);
#endif

#endif /* __PAGE_H__ */

