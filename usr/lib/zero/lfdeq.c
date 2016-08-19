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

static void
_lfdeqzeronode(struct lfdeqnode *node)
{
    tagptrzero(&node->prev);
    tagptrzero(&node->next);
#if (LFDEQMAPBUF)
    node->chain = NULL;
#endif

    return;
}

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
                m_syncwrite(&lfdeqbufchain, buf);
                buf = lfdeqallocpage();
            } else {
                m_syncwrite(&lfdeqbufchain, buf->next);
            }
        }
    } while (!buf);
    _lfdeqzeronode(buf);

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
lfdeqenqueue(struct lfdeq *lfdeq, LFDEQ_VAL_T val)
{
    TAGPTR_T          tail;
    TAGPTR_TAG_T      ttag;
    TAGPTR_TAG_T      tag;
    struct lfdeqnode *node;
    struct lfdeqnode *last;

    node = lfdeqalloc();
    node->val = val;
    do {
        tail = lfdeq->tail;
        tagptrgettag(tail, ttag);
        tagptrgetadr(tail, last);
        tag = ttag + 1;
        tagptrsetadr(last, node->next);
        tagptrsettag(tag, node->next);
        if (tagptrcmpswap(&lfdeq->tail, &tail, &node->next)) {
            tagptrsetadr(node, node->prev);
            tagptrsettag(ttag, node->prev);
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
    LFDEQ_VAL_T       val;
    TAGPTR_ADR_T      adr2;
    TAGPTR_TAG_T      htag;
    TAGPTR_TAG_T      ttag;
    TAGPTR_TAG_T      tag;
    struct lfdeqnode *hptr;
    struct lfqdenode *tptr;
    struct lfdeqnode *iptr;
    struct lfdeqnode *node;

    do {
        head = lfdeq->head;
        tagptrgetadr(head, hptr);
        tail = lfdeq->tail;
        item = hptr->prev;
        tagptrgetadr(tail, tptr);
        tagptrgetadr(item, iptr);
        val = hptr->val;
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
                    tagptrgetadr(tail, tptr);
#if (LFDEQMAPBUF)
                    node = lfdeqalloc();
#else
                    node = calloc(1, sizeof(struct lfdeqnode));
#endif
                    tag = ttag + 1;
                    node->val = LFDEQ_VAL_NONE;
                    tagptrsetadr(tptr, node->next);
                    tagptrsettag(tag, node->next);
                    tagptrsetadr(node, node->prev);
                    tagptrsettag(tag, node->prev);
                    if (tagptrcmpswap(&lfdeq->tail, &tail, &node->prev)) {
                        tagptrsettag(ttag, node->prev);
                        tagptrsetadr(node, hptr->prev);
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
                tagptrsettag(tag, item);
                if (tagptrcmpswap(&lfdeq->head, &head, &item)) {
                    val = hptr->val;
#if (LFDEQMAPBUF)
                    lfdeqfree(hptr);
#else
                    free(hptr);
#endif
                    
                    return val;
                }
            }
        } else if (hptr == tptr) {
                
            return NULL;
        } else {
            tagptrgettag(item, tag);
            tagptrgettag(head, htag);
            if (tag != htag) {
                _lfdeqfixqueue(lfdeq, head, tail);
                
                continue;
            }
            tag = htag + 1;
            tagptrsettag(tag, item);
            tagptrcmpswap(&lfdeq->head, &head, &item);
        }
    } while (1);

    /* NOTREACHED */
    return LFDEQ_VAL_NONE;
}

