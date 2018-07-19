#ifndef __MT_LFDEQ_H__
#define __MT_LFDEQ_H__

#include <stdint.h>
#include <string.h>
#include <mach/asm.h>

#define LFDEQ_MAX_ITEMS      32768
#define LFDEQ_STABLE         0
#define LFDEQ_PUSH_BIT       (1U << 15)
#define LFDEQ_ADR_LK_BIT_POS 0

struct lfdeqitem {
    struct lfdeqitem *prev;
    struct lfdeqitem *next;
    struct {
        int16_t       left;
        int16_t       right;
    } info;
    int32_t           _pad;
};

#define lfdeqislpush(item)  ((item)->info.left & LFDEQ_PUSH_BIT)
#define lfdeqisrpush(item)  ((item)->info.right & LFDEQ_PUSH_BIT)
#define lfdeqisstable(item) (!lfdeqislpush(item) && !lfdeqisrpush(item))
struct lfdeq {
    struct {
        int16_t   left;
        int16_t   right;
    } info;
};

extern struct lfdeqitem *lfdeqbase;
extern struct lfdeq      lfdeq;

int16_t lfdeqallocitem(void);

#define lfdeqsetdata(item, data, len) memcpy(item, data, len)

#if 0

static __inline__ void
lfdeqfixright(struct lfdeqitem *item)
{
    struct lfdeqitem *node;
    struct lfdeqitem *prev;
    struct lfdeqitem *next;
    int16_t           left;
    int16_t           right;

    right = item->info.right;
    *((uint32_t *)&node->info) = *((uint32_t *)&item->info);
    if (*((uint32_t *)&lfdeq.info) != *((uint32_t *)&item->info)) {

        return;
    }
    next = &lfdeqbase[right];
    node->info.right |= LFDEQ_PUSH_BIT;
    left = next->info.left;
    prev = &lfdeqbase[left];
    if (*((uint32_t *)&lfdeq.info) != *((uint32_t *)&node->info)) {

        return;
    }
    right = prev->next->info.right;
    prev->next->info.right |= LFDEQ_PUSH_BIT;
    if (prev->next != next) {
        if (*((uint32_t *)&lfdeq.info) != *((uint32_t *)&item->info)) {

            return;
        }
        if (m_cmpswap32((m_atomic32_t *)&prev->info,
                        (m_atomic32_t)prev->next->info,
                        (m_atomic32_t)prev->next->info | LFDEQ_PUSH_BIT)) {

            return;
        }
    }
}

static __inline__ void
lfdeqfix(struct lfdeqitem *item)
{
    if (lfdeqisrpush(item)) {
        lfdeqfixright(item);
    } else if (lfdeqislpush(item)) {
        lfdeqfixleft(item);
    }

    return;
}

#endif /* 0 */

static __inline__ void
lfdeqpushtail(struct lfdeq *deq, void *data, size_t len)
{
    struct lfdeqitem  head;
    int16_t           slot = lfdeqallocitem();
    struct lfdeqitem *item = &lfdeqbase[slot];

    lfdeqsetdata(item, data, len);
    do {
        *((uint32_t *)&head.info) = *((uint32_t *)&lfdeq.info);
        if (!item->info.right) {
            if (m_cmpswap32((m_atomic32_t *)&lfdeq.info,
                            (m_atomic32_t)&head.info,
                            (m_atomic32_t)&item->info)) {

                return;
            }
        } else if (lfdeqisstable(&lfdeq)) {
            item->info.left = lfdeq.info.right;
            item->info.right |= LFDEQ_PUSH_BIT;
            if (m_cmpswap32((m_atomic32_t *)&lfdeq.info,
                            (m_atomic32_t)&head.info,
                            (m_atomic32_t)&item->info)) {
                lfdeqfixright(item);

                return;
            }
        } else {
            lfdeqfix(*(uint32_t *)&head.info);
        }
    } while (1);

    return;
}

#endif /* __MT_LFDEQ_H__ */

