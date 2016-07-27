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
    TAGPTR_T  prev;
    TAGPTR_T  next;
    LFQ_VAL_T val;
};

struct lfq {
    TAGPTR_T       head;
    TAGPTR_T       tail;
    struct lfqnode dummy;
};

#endif /* __ZERO_LFQ_H__ */

