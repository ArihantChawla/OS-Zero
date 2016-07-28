#include <stdint.h>
#include <zero/asm.h>
#include <zero/atomic.h>
#include <zero/lfq.h>

void
lfqenqueue(struct lfq *q, struct lfqnode *node)
{
    TAGPTR_T    tail;
    TAGPTR_T    tmp;
    TAGPTR_T    src;
    TAGPTRTAG_T tag;

    do {
        tagptrinitadr(node, src);               // src.adr = node
        tail = q->tail;
        tag = tagptrgettag(tail);               // tag = tail.tag
        tagptrsettag(tag, src);                 // src.tag = tail.tag
        tag++;                                  // tag = tail.tag + 1;
        tagptrsettag(tag, tmp);                 // tmp.tag = tail.tag + 1
        node->next = tmp;
        if (tagptrcmpswap(&q->tail, &tail, &tmp)) {
            ((struct lfqnode *)tagptrgetadr(tail))->prev = src;

            return;
        }
    } while (1);        

    return;
}

