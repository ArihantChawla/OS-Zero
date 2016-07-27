#ifndef __ZERO_LFQ_H__
#define __ZERO_LFQ_H__

/*
 * lock-free dual-ended queues
 *
 * REFERENCE: https://www.offblast.org/stuff/books/FIFO_Queues.pdf
 */

#if !defined(LFQ_VAL_T)
#include <stdint.h>
#define LFQ_VAL_T uintptr_t
#endif

#include <zero/tagptr.h>

struct lfqnode {
    LFQ_VAL_T     val;
    struct tagptr prev;
    struct tagptr next;
};

struct lfq {
    struct tagptr  head;
    struct tagptr  tail;
    struct lfqnode dummy;
};

#endif /* __ZERO_LFQ_H__ */

