#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <stdint.h>
#include <kern/conf.h>
#include <kern/perm.h>

/* API */
void *bufalloc(void);

#define devgetblk(buf, blk) devfindblk(buf, blk, 0)

#define BUFSIZE     (1UL << BUFSIZELOG2)
#define BUFSIZELOG2 13                          // TODO: make this per-device

#define BUFNEVICT   8
#define BUFNBYTE    (32768 * 1024)
#define BUFNBLK     (BUFNBYTE >> BUFSIZELOG2)

#define BUFNET      (1U << (PERMNBIT + 1))      // precalculate checksum
#define BUFNOLK     (1U << (PERMNBIT + 2))      // don't lock on access

#if (BUFMULTITAB)

#define NLVL0BIT    16
#define NLVL1BIT    16
#define NLVL2BIT    16
#define NLVL3BIT    16
#define NLVL0BLK    (1UL << NLVL0BIT)
#define NLVL1BLK    (1UL << NLVL1BIT)
#define NLVL2BLK    (1UL << NLVL2BIT)
#define NLVL3BLK    (1UL << NLVL3BIT)

#define bufkey0(num)                                                    \
    (((num) >> (NLVL1BIT + NLVL2BIT + NLVL3BIT)) & ((1UL << NLVL0BIT) - 1))
#define bufkey1(num)                                                    \
    (((num) >> (NLVL2BIT + NLVL3BIT)) & ((1UL << NLVL1BIT) - 1))
#define bufkey2(num)                                                    \
    (((num) >> NLVL3BIT) & ((1UL << NLVL2BIT) - 1))
#define bufkey3(num)                                                    \
    ((num) & ((1UL << NLVL3BIT) - 1))

/* table item */
struct buftab {
    void *ptr;                  // pointer to next table or data
    long  nref;                 // # of references to table items
};

struct devbuf {
    long           lk;          // mutex
    struct buftab  tab;         // multi-level table of blocks
};

#else /* !BUFMULTITAB */

#define BUFNHASHITEM 65536
#define bufkey(num) ((num) & (BUFNHASHITEM - 1))

#endif /* BUFMULTITAB */

/* type values */
#define BUFUNUSED 0x00
#define BUFSUPER  0x01
#define BUFINODE  0x02
#define BUFDIR    0x03
#define BUFFILE   0x04
#define BUFPIPE   0x05
struct bufblk {
    long           type;
    long           dev;         // device #
    long           num;         // per-device block #
    long           flg;         // block flag-bits
    long           nb;          // # of bytes
    void          *data;        // in-core block data
    struct bufblk *prev;        // previous in LRU queue
    struct bufblk *next;        // next in LRU queue
};

struct bufblkq {
    volatile long  lk;
    struct bufblk *head;
    struct bufblk *tail;
    long           pad;
};

/* push bp in front of LRU queue */
#define bufpushqblk(qp, bp)                                             \
    do {                                                                \
        struct bufblk *_head;                                           \
        struct bufblk *_tail;                                           \
                                                                        \
        (bp)->prev = NULL;                                              \
        (bp)->next = NULL;                                              \
        mtxlk(&(qp)->lk);                                               \
        _head = (qp)->head;                                             \
        _tail = (qp)->tail;                                             \
        if (_head) {                                                    \
            _head->prev = (bp);                                         \
            if (!_tail) {                                               \
                (qp)->tail = _head;                                     \
            }                                                           \
            (bp)->next = _head;                                         \
        }                                                               \
        (qp)->head = (bp);                                              \
        mtxunlk(&(qp)->lk);                                             \
    } while (0)

/* remove block from the front of LRU queue */
#define bufpopqblk(qp, bp)                                              \
    do {                                                                \
        struct bufblk *_head;                                           \
        struct bufblk *_tail;                                           \
                                                                        \
        mtxlk(&(qp)->lk);                                               \
        _head = (qp)->head;                                             \
        _tail = (qp)->tail;                                             \
        if (_head) {                                                    \
            if (_head->next) {                                          \
                _head->next->prev = NULL;                               \
            }                                                           \
            if (_head->next == _tail) {                                 \
                (qp)->tail = NULL;                                      \
            }                                                           \
            (qp)->head = _head->next;                                   \
            (bp) = _head;                                               \
        }                                                               \
        mtxunlk(&(qp)->lk);                                             \
} while (0)

/* add block to the back of LRU queue */
#define bufqblk(qp, bp)                                                 \
    do {                                                                \
        struct bufblk *_tail;                                           \
                                                                        \
        (bp)->next = NULL;                                              \
        mtxlk(&(qp)->lk);                                               \
        _tail  = (qp)->tail;                                            \
        (bp)->prev = _tail;                                             \
        if (_tail) {                                                    \
            (qp)->tail = (bp);                                          \
        } else {                                                        \
            (bp)->prev = NULL;                                          \
            (qp)->head = (bp);                                          \
        }                                                               \
        mtxunlk(&(qp)->lk);                                             \
    } while (0)

/* remove block from the back of LRU queue, store pointer to bp */
#define bufdeqblk(qp, bp)                                               \
    do {                                                                \
        struct bufblk *_head;                                           \
        struct bufblk *_tail;                                           \
                                                                        \
        mtxlk(&(qp)->lk);                                               \
        _head = (qp)->head;                                             \
        _tail = (qp)->tail;                                             \
        if (_tail) {                                                    \
            if (_tail->prev) {                                          \
                _tail->prev->next = NULL;                               \
            }                                                           \
            if (_tail->prev == _head) {                                 \
                (qp)->tail = NULL;                                      \
            } else {                                                    \
                (qp)->tail = _tail->prev;                               \
            }                                                           \
            (bp) = _tail;                                               \
        } else {                                                        \
            (bp) = (qp)->head;                                          \
            (qp)->head = NULL;                                          \
        }                                                               \
        mtxunlk(&(qp)->lk);                                             \
    } while (0)

/* remove block bp from LRU queue */
#define bufrmqblk(qp, bp)                                               \
    do {                                                                \
        struct bufblk *_prev = (bp)->prev;                              \
        struct bufblk *_next = (bp)->next;                              \
                                                                        \
        mtxlk(&(qp)->lk);                                               \
        _prev = (bp)->prev;                                             \
        _next = (bp)->next;                                             \
        if (_prev) {                                                    \
            _prev->next = _next;                                        \
        } else {                                                        \
            (qp)->head = _next;                                         \
        }                                                               \
        if (_next) {                                                    \
            _next->prev = _prev;                                        \
        }                                                               \
        mtxunlk(&(qp)->lk);                                             \
    } while (0)
        
#endif /* __KERN_IO_BUF_H__ */

