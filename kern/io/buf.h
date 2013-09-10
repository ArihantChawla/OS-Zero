#ifndef __KERN_IO_BUF_H__
#define __KERN_IO_BUF_H__

#include <stdint.h>
#include <kern/perm.h>

/* API */
void *bufalloc(void);

typedef uint64_t blkid_t;

#define devgetblk(buf, blk) devfindblk(buf, blk, 0)

#define BUFSIZE     (1UL << BUFSIZELOG2)
#define BUFSIZELOG2 16

#define BUFNEVICT   8
#define BUFNBYTE    (32768 * 1024)
#define BUFNBLK     (BUFNBYTE >> BUFSIZELOG2)

#define BUFNET      (1U << (PERMNBIT + 1))      // precalculate checksum
#define BUFNOLK     (1U << (PERMNBIT + 2))      // don't lock on access

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

struct bufblk {
    blkid_t        blknum;      // per-device block #
    long           devnum;      // device #
    long           flg;         // block flag-bits
    long           nb;          // # of bytes
    void          *data;        // in-core block data
    struct bufblk *prev;        // previous in LRU queue
    struct bufblk *next;        // next in LRU queue
#if (LONGSIZE == 8)
    long           pad;         // pad to 8 long-words
#endif
};

struct bufblkq {
    struct bufblk *head;
    struct bufblk *tail;
};

/* push bp in front of LRU queue */
#define bufpushqblk(qp, bp)                                             \
    do {                                                                \
        struct bufblk *_head = (qp)->head;                              \
        struct bufblk *_tail = (qp)->tail;                              \
                                                                        \
        (bp)->prev = NULL;                                              \
        (bp)->next = NULL;                                              \
        if (_head) {                                                    \
            _head->prev = (bp);                                         \
            if (!_tail) {                                               \
                (qp)->tail = _head;                                     \
            }                                                           \
            (bp)->next = _head;                                         \
        }                                                               \
        (qp)->head = (bp);                                              \
    } while (0)

/* remove block from the front of queue */
#define bufpopqblk(qp, bp)                                              \
    do {                                                                \
        struct bufblk *_head = (qp)->head;                              \
        struct bufblk *_tail = (qp)->tail;                              \
                                                                        \
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
} while (0)

/* remove block from the back of LRU queue, store pointer to bp */
#define bufdeqblk(qp, bp)                                               \
    do {                                                                \
        struct bufblk *_head = (qp)->head;                              \
        struct bufblk *_tail = (qp)->tail;                              \
                                                                        \
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
    } while (0)

/* remove block bp from LRU queue */
#define bufrmqblk(qp, bp)                                               \
    do {                                                                \
        struct bufblk *_prev = (bp)->prev;                              \
        struct bufblk *_next = (bp)->next;                              \
                                                                        \
        if (_prev) {                                                    \
            _prev->next = _next;                                        \
        } else {                                                        \
            (qp)->head = _next;                                         \
        }                                                               \
        if (_next) {                                                    \
            _next->prev = _prev;                                        \
        }                                                               \
    } while (0)
        
#endif /* __KERN_IO_BUF_H__ */

