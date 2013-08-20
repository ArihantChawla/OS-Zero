#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>

#include <zero/trix.h>
#define __KERNEL__ 1
#include <zero/mtx.h>

#define pagenum(adr)       ((adr) >> PAGESIZELOG2)
#define swapblknum(sp, pg) ((pg) - (sp)->pgtab)
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

#define pagegetqid(pg) (tzerol(pg->nflt))
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
      
#define pagepush(pq, pg)                                                \
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
        long          _pqid;                                            \
        struct pageq *_pq;                                              \
        struct page  *_tmp;                                             \
                                                                        \
        _pqid = pagegetqid(pg);                                         \
        mtxlk(&_pg->lk);                                                \
        _tmp = (pg)->prev;                                              \
        if (_tmp) {                                                     \
            _tmp->next = (pg)->next;                                    \
        } else {                                                        \
            _tmp = (pg)->next;                                          \
            _pq->head = _tmp;                                           \
            if (_tmp) {                                                 \
                _tmp->prev = (pg)->prev;                                \
            } else {                                                    \
                _pq->tail = _tmp;                                       \
            }                                                           \
            _pq->head = _tmp;                                           \
        }                                                               \
        _tmp = (pg)->next;                                              \
        if (_tmp) {                                                     \
            _tmp->prev = (pg)->prev;                                    \
        } else {                                                        \
            _tmp = (pg)->prev;                                          \
            _pq->tail = _tmp;                                           \
            if (_tmp) {                                                 \
                _tmp->next = NULL;                                      \
            } else {                                                    \
                _pq->head = _pq->tail = _tmp;                           \
            }                                                           \
        }                                                               \
        mtxunlk(&_pq-lk);                                               \
    } while (0)

void         pageinitzone(uintptr_t base, struct pageq *zone, unsigned long nb);
void         pageinit(uintptr_t base, unsigned long nb);
struct page *pagealloc(void);
void         pagefree(void *adr);
#if 0
void         pagefree(void *adr);
void         swapfree(uintptr_t adr);
#endif

#endif /* __PAGE_H__ */

