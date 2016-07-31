#ifndef __ZERO_LFQ_H__
#define __ZERO_LFQ_H__

/*
 * lock-free dual-ended queues
 *
 * REFERENCE: https://www.offblast.org/stuff/books/FIFO_Queues.pdf
 */

#include <stdint.h>
#if !defined(LFQ_VAL_T)
#define LFQ_VAL_T    uintptr_t
#define LFQ_VAL_NONE (~(uintptr_t)0)
#endif
#include <zero/param.h>
#include <zero/tagptr.h>

struct lfqnode {
    TAGPTR_T  prev;
    TAGPTR_T  next;
    LFQ_VAL_T val;
    uint8_t   _pad[CLSIZE
                   - 2 * sizeof(TAGPTR_T)
                   - sizeof(LFQ_VAL_T)];
};

struct lfq {
    struct lfqnode dummy;
    TAGPTR_T       head;
    TAGPTR_T       tail;
    uint8_t        _pad[2 * CLSIZE
                        - sizeof(struct lfqnode)
                        - 2 * sizeof(TAGPTR_T)];
};

void      lfqinitqueue(struct lfq *lfq);
void      lfqenqueue(struct lfq *lfq, struct lfqnode *node);
LFQ_VAL_T lfqdequeue(struct lfq *lfq);

#endif /* __ZERO_LFQ_H__ */

