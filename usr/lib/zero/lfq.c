#include <stdint.h>
#include <zero/asm.h>
#include <zero/atomic.h>
#include <zero/lfq.h>

void
lfqqueue(struct lfq *q, struct lfqnode *node)
{
    TAGPTR_T tail;
    TAGPTR_T tmp;
    TAGPTR_T src;

    do {
        tagptrinitadr(node, src);               // src.adr = node
        tail = q->tail;
        tagptrcpytag(tail, src);                // src.tag = tail.tag
        tmp = src;
        tagptrinctag(tmp);                      // tmp.tag = tail.tag + 1
        node->next = tmp;
        if (tagptrcmpswap(&q->tail, &tail, &tmp)) {
            ((struct lfqnode *)tagptrgetadr(tail))->prev = src;

            return;
        }
    } while (1);        

    return;
}

