#include <stdint.h>
#include <zero/asm.h>
#include <zero/atomic.h>
#include <zero/lfq.h>

/* REFERENCE: https://www.offblast.org/stuff/books/FIFO_Queues.pdf */

static void
_lfqfixqueue(struct lfq *lfq, TAGPTR_T head, TAGPTR_T tail)
{
    TAGPTR_T        cur;
    TAGPTR_T        prev;
    TAGPTR_T        next;
    TAGPTR_T        tmp;
    TAGPTR_TAG_T    tag1;
    TAGPTR_TAG_T    tag2;
    struct lfqnode *node;

    cur = tail;
    do {
        tmp = lfq->head;
        while ((tagptrcmp(&head, &tmp)) && (!tagptrcmp(&cur, &head))) {
            next = ((struct lfqnode *)tagptrgetadr(cur))->next;
            tag1 = tagptrgettag(cur);
            tag2 = tagptrgettag(next);
            if (tag1 != tag2) {
                
                return;
            } else {
                    node = ((struct lfqnode *)tagptrgetadr(next));
                    tag1--;
                    prev = node->prev;
                    tagptrsettag(tag1, cur);
                    if (!tagptrcmp(&prev, &cur)) {
                        node->prev = cur;
                    }
            }
        }
    } while (1);

    return;
}

void
lfqinitqueue(struct lfq *lfq)
{
    TAGPTR_T dummy;

    tagptrinitadr(&lfq->dummy, dummy);
    lfq->head = dummy;
    lfq->tail = dummy;
}

void
lfqenqueue(struct lfq *lfq, struct lfqnode *node)
{
    TAGPTR_T     tail;
    TAGPTR_T     tmp;
    TAGPTR_T     src;
    TAGPTR_TAG_T tag;

    do {
        tagptrinitadr(node, src);               // src.adr = node
        tail = lfq->tail;
        tag = tagptrgettag(tail);               // tag = tail.tag
        tagptrsettag(tag, src);                 // src.tag = tail.tag
        tag++;                                  // tag = tail.tag + 1;
        tagptrsettag(tag, tmp);                 // tmp.tag = tail.tag + 1
        node->next = tmp;
        if (tagptrcmpswap(&lfq->tail, &tail, &tmp)) {
            ((struct lfqnode *)tagptrgetadr(tail))->prev = src;

            return;
        }
    } while (1);        

    return;
}

LFQ_VAL_T
lfqdequeue(struct lfq *lfq)
{
    TAGPTR_T        head;
    TAGPTR_T        tail;
    TAGPTR_T        dummy;
    LFQ_VAL_T       val;
    TAGPTR_ADR_T    adr1;
    TAGPTR_ADR_T    adr2;
    TAGPTR_TAG_T    tag1;
    TAGPTR_TAG_T    tag2;
    struct lfqnode *item;
    struct lfqnode  node;

    do {
        dummy = *((TAGPTR_T *)&lfq->dummy);
        head = lfq->head;
        item = tagptrgetadr(dummy);
        tail = lfq->tail;
        val = item->val;
        if (tagptrcmp(&head, &lfq->head)) {
            if (val != LFQ_VAL_NONE) {
                if (!tagptrcmp(&tail, &head)) {
                    tag1 = tagptrgettag(head);
                    tag2 = tagptrgettag(dummy);
                    if (tag1 != tag2) {
                        _lfqfixqueue(lfq, head, tail);

                        continue;
                    }
                } else {
                    tag1 = tagptrgettag(tail);
                    node.val = LFQ_VAL_NONE;
                    tag2 = tag1 + 1;
                    tagptrinitadr(&node, head);
                    tagptrinitadr(&node, dummy);
                    tagptrsettag(tag2, head);
                    tagptrsettag(tag1, dummy);
                    node.next = tail;
                    if (tagptrcmpswap(&lfq->tail, &tail, &head)) {
                        lfq->dummy.prev = dummy;
                    }

                    continue;
                }
                tag1 = tagptrgettag(dummy);
                tag1++;
                tagptrsettag(tag1, dummy);
                if (tagptrcmpswap(&lfq->head, &head, &dummy)) {

                    return val;
                }
            } else {
                adr1 = tagptrgetadr(head);
                adr2 = tagptrgetadr(tail);
                if (adr1 == adr2) {

                    return LFQ_VAL_NONE;
                } else {
                    tag2 = tagptrgettag(head);
                    tag1 = tagptrgettag(dummy);
                    if (tag2 != tag1) {
                        _lfqfixqueue(lfq, head, tail);

                        continue;
                    } else {
                        tag2++;
                        tagptrsettag(tag2, dummy);
                        tagptrcmpswap(&lfq->head, &head, &dummy);
                    }
                }
            }
        }
    } while (1);

    /* NOTREACHED */
    return LFQ_VAL_NONE;
}

