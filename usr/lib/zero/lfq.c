#include <zero/atomic.h>
#include <zero/lfq.h>

void
lfqqueue(struct lfq *q, struct lfqnode *node)
{
    struct tagptr tail;
    struct tagptr tag;
    struct tagptr tptr;
    struct tagptr src;

    do {
//        src = *node;
        src.data.adr = node;
        tptr.data.adr = node;
        tail = q->tail;
        src.data.tag = tail.data.tag;
        tptr.data.tag = tail.data.tag + 1;
        tag.data.adr = tail.data.adr;
        tag.data.tag = tptr.data.tag;
        node->next = tag;
        if (m_cmpswapdbl((volatile long *)&q->tail,
                         (volatile long *)&tail,
                         tptr)) {
            ((struct lfqnode *)tail.data.adr)->prev = src;
        }
    } while (1);

    return;
}

