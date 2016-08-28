#ifndef __ZERO_LFDEQ_H__
#define __ZERO_LFDEQ_H__

#include <zero/param.h>
#include <zero/trix.h>

#define LFDEQMAPBUF   1
#if (LFDEQMAPBUF)
#define LFDEQLKBITPOS 0
#define LFDEQLKBIT    (1UL << LFDEQLKBITPOS)
#include <zero/asm.h>
#define _lfdeqtrylkchain(ptr)                                           \
    (!m_cmpsetbit((volatile long *)(ptr), LFDEQLKBITPOS))
#define _lfdequnlkchain(ptr)                                            \
    (m_cmpclrbit((volatile long *)(ptr), LFDEQLKBITPOS))
#endif

/*
 * lock-free dual-ended queues
 *
 * REFERENCE: https://www.offblast.org/stuff/books/FIFO_Queues.pdf
 */

#include <stdint.h>
#if !defined(LFDEQ_VAL_T)
#define LFDEQ_VAL_T    uintptr_t
#define LFDEQ_VAL_NONE 0
#endif
#include <zero/param.h>
#include <zero/tagptr.h>

struct lfdeqnode {
    TAGPTR_T          prev;
    TAGPTR_T          next;
#if (LFDEQMAPBUF)
    struct lfdeqnode *chain;
#endif
    LFDEQ_VAL_T       val;
};
#define LFDEQNODESIZE rounduppow2(sizeof(struct lfdeqnode), CLSIZE)

struct lfdeq {
    TAGPTR_T         head;
    TAGPTR_T         tail;
    struct lfdeqnode dummy;
    long             nitem;
};
#define LFDEQSIZE rounduppow2(sizeof(struct lfdeq), CLSIZE)

void        lfdeqinitqueue(struct lfdeq *lfdeq);
void        lfdeqenqueue(struct lfdeq *lfdeq, LFDEQ_VAL_T val);
LFDEQ_VAL_T lfdeqdequeue(struct lfdeq *lfdeq);

#endif /* __ZERO_LFDEQ_H__ */

