#include <zero/atomic.h>
#include <zero/lfq.h>

void
lfqqueue(struct lfq *q, struct lfqnode *node)
{
    struct tagptr tail;
    struct tagptr tag;
    struct tagptr tptr;
    struct lfqstr src;

    do {
        src = *node;
        src.ptr = node;
        tptr.ptr = node;
        tail = q->tail;
        src.tag = tail.tag;
        tptr.tag = tail.tag + 1;
        tag.ptr = tail.ptr;
        tag.tag = tptr.tag;
        node->next = tag;
        if (m_cmpswapdbl((volatile long *)&q->tail,
                         (volatile long *)&tail,
                         tptr)) {
            tail.ptr->prev = *src;
        }
    } while (1);

    return;
}

