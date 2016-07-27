#ifndef __ZERO_LFQ_H__
#define __ZERO_LFQ_H__

/*
 * lock-free dual-ended queues
 *
 * REFERENCE: https://www.offblast.org/stuff/books/FIFO_Queues.pdf
 */

#include <zero/tagptr.h>

struct lfqnode {
    LFQVAL_T      val;
    struct tagptr prev;
    struct tagptr next;
};

struct lfq {
    struct tagptr  head;
    struct tagptr  tail;
    struct lfqnode dummy;
};

#endif /* __ZERO_LFQ_H__ */

