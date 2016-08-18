#include <stdlib.h>
#include <stdint.h>
#include <zero/asm.h>
#include <zero/atomic.h>
#include <zero/tagptr.h>
#include <zero/lfdeq.h>
#if (LFDEQMAPBUF)
#define MMAP_DEV_ZERO 0
#include <unistd.h>
#include <zero/param.h>
#include <zero/unix.h>
#endif

/* REFERENCE: https://www.offblast.org/stuff/books/FIFO_Queues.pdf */

#if (LFDEQMAPBUF)

static struct lfdeqnode *lfdeqbufchain;

static struct lfdeqnode *
lfdeqallocpage(void)
{
    struct lfdeqnode *buf = mapanon(0, PAGESIZE);
    long              n = PAGESIZE / LFDEQNODESIZE;
    struct lfdeqnode *cur;
    uintptr_t         up;
    uintptr_t         lock;

    if (buf == MAP_FAILED) {
        abort();
    }
    cur = &buf[n - 1];
    while (--n) {
        do {
            lock = _lfdeqtrylkchain(&lfdeqbufchain);
            if (!lock) {
                usleep(100);
            }
        } while (!lock);
        up = (uintptr_t)lfdeqbufchain;
        up &= ~LFDEQLKBIT;
        cur->chain = (void *)up;
        m_syncwrite(&lfdeqbufchain, cur);
        cur--;
    }
    cur->chain = NULL;

    return cur;
}

static struct lfdeqnode *
lfdeqalloc(void)
{
    struct lfdeqnode *buf = NULL;
    uintptr_t         up;
    
    do {
        if (_lfdeqtrylkchain(&lfdeqbufchain)) {
            up = (uintptr_t)lfdeqbufchain;
            buf = (void *)(up & ~LFDEQLKBIT);
            if (!buf) {
                buf = lfdeqallocpage();
            } else {
                m_syncwrite(&lfdeqbufchain, buf->next);
            }
        }
    } while (!buf);

    return buf;
}

static void
lfdeqfree(struct lfdeqnode *node)
{
    uintptr_t up;
    uintptr_t lock;

    do {
        lock = _lfdeqtrylkchain(&lfdeqbufchain);
        if (!lock) {
            usleep(100);
        }
    } while (!lock);
    up = (uintptr_t)lfdeqbufchain;
    up &= ~LFDEQLKBIT;
    node->chain = (void *)up;
    m_syncwrite(&lfdeqbufchain, node);

    return;
}

#endif /* LFDEQMAPBUF */

static void
_lfdeqfixqueue(struct lfdeq *lfdeq, TAGPTR_T head, TAGPTR_T tail)
{
    TAGPTR_T          cur;
    TAGPTR_T          prev;
    TAGPTR_T          next;
    TAGPTR_T          tmp;
    TAGPTR_TAG_T      tag1;
    TAGPTR_TAG_T      tag2;
    TAGPTR_TAG_T      tag3;
    struct lfdeqnode *node1;
    struct lfdeqnode *node2;
    struct lfdeqnode *node3;

    cur = tail;
    do {
        tmp = lfdeq->head;
        tagptrgetadr(tmp, node1);
        next = node1->next;
        tagptrgettag(cur, tag1);
        while ((tagptrcmp(&head, &lfdeq->head)) && (!tagptrcmp(&cur, &head))) {
            tag3 = tag1;
            tagptrgetadr(cur, node2);
            next = node2->next;
            tagptrgetadr(next, node3);
            tagptrgettag(next, tag2);
            if (tag1 != tag2) {
                
                return;
            }
            tagptrgetadr(cur, node1);
            tag3--;
            prev = node1->prev;
            tagptrsettag(tag3, cur);
            if (!tagptrcmp(&prev, &cur)) {
                node3->prev = cur;
            }
        }
        tag1--;
        tagptrsetadr(node3, cur);
        tagptrsettag(tag1, cur);
    } while (1);

    return;
}

void
lfdeqinitqueue(struct lfdeq *lfdeq)
{
    TAGPTR_T dummy;

    tagptrsetadr(&lfdeq->dummy, lfdeq->head);
    tagptrsetadr(&lfdeq->dummy, lfdeq->tail);

    return;
}

void
lfdeqenqueue(struct lfdeq *lfdeq, struct lfdeqnode *node)
{
    TAGPTR_T          tail;
    TAGPTR_TAG_T      ttag;
    TAGPTR_TAG_T      tag;
    struct lfdeqnode *last;

    do {
        tail = lfdeq->tail;
        tagptrgettag(tail, ttag);
        tag = ttag + 1;
        tagptrgetadr(tail, last);
        tagptrsetadr(last, node->next);
        tagptrsettag(tag, node->next);
        if (tagptrcmpswap(&lfdeq->tail, &tail, &node->next)) {
            tagptrsettag(ttag, last->prev);
            tagptrsetadr(node, last->prev);

            return;
        }
    } while (1);        

    return;
}

LFDEQ_VAL_T
lfdeqdequeue(struct lfdeq *lfdeq)
{
    TAGPTR_T          item;
    TAGPTR_T          head;
    TAGPTR_T          tail;
    TAGPTR_T          tmp;
    LFDEQ_VAL_T       val;
    TAGPTR_ADR_T      adr1;
    TAGPTR_ADR_T      adr2;
    TAGPTR_TAG_T      htag;
    TAGPTR_TAG_T      ttag;
    TAGPTR_TAG_T      tag;
    struct lfdeqnode *ptr1;
    struct lfdeqnode *ptr2;
    struct lfdeqnode *node;

    do {
        head = lfdeq->head;
        tagptrgetadr(head, ptr1);
        tail = lfdeq->tail;
        item = ptr1->prev;
        tagptrgetadr(item, ptr2);
        val = ptr1->val;
        if (tagptrcmp(&head, &lfdeq->head)) {
            if (val != LFDEQ_VAL_NONE) {
                tagptrgettag(head, htag);
                tagptrgettag(tail, ttag);
                if (!tagptrcmp(&tail, &head)) {
                    tagptrgettag(item, tag);
                    if (htag != tag) {
                        _lfdeqfixqueue(lfdeq, head, tail);
                        
                        continue;
                    }
                } else {
                    tagptrgetadr(tail, adr1);
#if (LFDEQMAPBUF)
                    node = lfdeqalloc();
#else
                    node = calloc(1, sizeof(struct lfdeqnode));
#endif
                    tag = ttag + 1;
                    node->val = LFDEQ_VAL_NONE;
                    tagptrsetadr(adr1, node->next);
                    tagptrsettag(tag, node->next);
                    if (tagptrcmpswap(&lfdeq->tail, &tail, &node)) {
                        tagptrsettag(ttag, item);
                    } else {
#if (LFDEQMAPBUF)
                        lfdeqfree(node);
#else
                        free(node);
#endif
                    }
                    
                    continue;
                }
                tag = htag + 1;
                tagptrsetadr(ptr2, tmp);
                tagptrsettag(tag, tmp);
                if (tagptrcmpswap(&lfdeq->head, &head, &tmp)) {
#if (LFDEQMAPBUF)
                    lfdeqfree(ptr1);
#else
                    free(ptr1);
#endif
                    
                    return val;
                }
            } else {
                tagptrgetadr(head, adr1);
                tagptrgetadr(tail, adr2);
                if (adr1 == adr2) {
                    
                    return NULL;
                } else {
                    tagptrgettag(item, tag);
                    tagptrgettag(head, htag);
                    if (tag != htag) {
                        _lfdeqfixqueue(lfdeq, head, tail);
                        
                        continue;
                    } else {
                        tag = htag + 1;
                        tagptrsetadr(ptr2, tmp);
                        tagptrsettag(tag, tmp);
                        tagptrcmpswap(&lfdeq->head, &head, &tmp);
                    }
                }
            }
        }
    } while (1);

    /* NOTREACHED */
    return LFDEQ_VAL_NONE;
}

