#include <stdlib.h>
#include <stdint.h>
#include <mach/asm.h>
#include <mach/atomic.h>
#include <mach/tagptr.h>
#include <zero/lfdeq.h>
#if (LFDEQMAPBUF)
#define MMAP_DEV_ZERO 0
#include <unistd.h>
#include <mach/param.h>
#include <zero/unix.h>
#endif

/* REFERENCE: https://www.offblast.org/stuff/books/FIFO_Queues.pdf */

#if (LFDEQMAPBUF)

static struct lfdeqnode *lfdeqbufchain;

static void
_lfdeqinitnode(struct lfdeqnode *node)
{
    tagptrzero(&node->prev);
    tagptrzero(&node->next);
#if (LFDEQMAPBUF)
    node->chain = NULL;
#endif
    node->val = LFDEQ_VAL_NONE;

    return;
}

static struct lfdeqnode *
_lfdeqallocpage(void)
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
_lfdeqalloc(void)
{
    struct lfdeqnode *buf = NULL;
    uintptr_t         up;

    do {
        if (_lfdeqtrylkchain(&lfdeqbufchain)) {
            up = (uintptr_t)lfdeqbufchain;
            buf = (void *)(up & ~LFDEQLKBIT);
            if (!buf) {
                m_syncwrite(&lfdeqbufchain, buf);
                buf = _lfdeqallocpage();
            } else {
                m_syncwrite(&lfdeqbufchain, buf->next);
            }
        }
    } while (!buf);
    _lfdeqinitnode(buf);

    return buf;
}

static void
_lfdeqfree(struct lfdeqnode *node)
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

void
lfdeqinitqueue(struct lfdeq *lfdeq)
{
    tagptrsetadr(lfdeq->head, &lfdeq->dummy);
    tagptrsetadr(lfdeq->tail, &lfdeq->dummy);
    tagptrzero(&lfdeq->dummy);
    lfdeq->nitem = 0;

    return;
}

static void
_lfdeqfix(struct lfdeq *lfdeq, TAGPTR_T tail, TAGPTR_T head)
{
    TAGPTR_T          cur;
    TAGPTR_T          prev;
    TAGPTR_T          next;
    struct tagptr    *tpptr1;
    struct tagptr    *tpptr2;
    struct lfdeqnode *node;

    cur = tail;
    while (tagptrcmp(&head, &lfdeq->head) && cur != head) {
        tpptr1 = (struct tagptr *)&cur;
        node = tpptr1->adr;
        next = node->next;
        prev = node->prev;
        tpptr2 = (struct tagptr *)&next;
        if (tpptr2->tag != tpptr1->tag) {

            return;
        }
        tpptr2 = (struct tagptr *)&next;
        tpptr1->tag--;
        if (!tagptrcmp(&prev, &cur)) {
            node = tpptr2->adr;
            node->prev = cur;
        }
        tpptr2->tag--;
        cur = next;
    }
}

LFDEQ_VAL_T
lfdeqenqueue(struct lfdeq *lfdeq, LFDEQ_VAL_T val)
{
    TAGPTR_T          tail;
    TAGPTR_T          prev;
    TAGPTR_T          next;
    struct tagptr    *tpptr;
    struct lfdeqnode *node;

    node = _lfdeqalloc();
    if (!node) {

        return LFDEQ_VAL_NONE;
    }
    node->val = val;
    do {
        tail = lfdeq->tail;
        prev = tail;
        tpptr = (struct tagptr *)&prev;
        tpptr->adr = node;
        tpptr = (struct tagptr *)&tail;
        tpptr->tag++;           // tpptr->tag = tail.tag + 1
        next = tail;            // next = { tail.adr, tail.tag + 1 }
        node->next = tail;      // node->next = { tail.adr, tail.tag + 1 }
        tpptr = (struct tagptr *)&next;
        tpptr->adr = node;      // next = { node, tail.tag + 1 }
        tpptr = (struct tagptr *)&tail;
        node = tpptr->adr;
        if (tagptrcmpswap(&lfdeq->tail, tail, next)) {
//            m_atomswap(&node->prev, prev);
            node->prev = prev;

            break;
        }
    } while (1);

    return val;
}

LFDEQ_VAL_T
lfdeqdequeue(struct lfdeq *lfdeq)
{
    LFDEQ_VAL_T       val;
    TAGPTR_T          head;
    TAGPTR_T          tail;
    TAGPTR_T          prev;
    TAGPTR_T          next;
    struct tagptr    *tpptr1;
    struct tagptr    *tpptr2;
    struct lfdeqnode *node;
    struct lfdeqnode *dummy;

    do {
        head = lfdeq->head;
        tail = lfdeq->tail;
        tpptr1 = (struct tagptr *)&head;
        node = tpptr1->adr;
        val = node->val;
        tpptr2 = (struct tagptr *)&tail;
        prev = node->prev;
        if (val != LFDEQ_VAL_NONE) {
            tpptr2 = (struct tagptr *)&prev;
            if (!tagptrcmp(&tail, &head)) {
                if (tpptr2->tag != tpptr1->tag) {
                    _lfdeqfix(lfdeq, tail, head);

                    continue;
                }
            } else {
                dummy = _lfdeqalloc();
                if (!dummy) {

                    return LFDEQ_VAL_NONE;
                }
                tpptr1 = (struct tagptr *)&tail;
                prev = tail;
                tpptr1->tag++;
                tpptr2 = (struct tagptr *)&prev;
                dummy->next = tail;
                tpptr1->adr = (TAGPTR_ADR_T)dummy;
                tpptr2->adr = (TAGPTR_ADR_T)dummy;
                tpptr1 = (struct tagptr *)&head;
                if (tagptrcmpswap(&lfdeq->tail, tail, prev)) {
                    node->prev = prev;
                } else {
#if (LFDEQMAPBUF)
                    _lfdeqfree(node);
#else
                    free(node);
#endif
                }

                continue;
            }
            tpptr1 = (struct tagptr *)&prev;
            node = tpptr1->adr;
            tpptr1->tag++;
            if (tagptrcmpswap(&lfdeq->head, head, prev)) {
#if (LFDEQMAPBUF)
                _lfdeqfree(node);
#else
                free(node);
#endif
                return val;
            }
        } else if (tpptr2->adr != tpptr1->adr) {

            return LFDEQ_VAL_NONE;
        } else {
            tpptr2 = (struct tagptr *)&prev;
            if (tpptr2->tag != tpptr1->tag) {
                _lfdeqfix(lfdeq, tail, head);

                continue;
            }
            tpptr2->tag++;
            tagptrcmpswap(&lfdeq->head, head, prev);
        }
    } while (1);
}

#if (LFDEQTEST)
int
main(int argc, char *argv[])
{
    ;
}

#endif /* 0 */

