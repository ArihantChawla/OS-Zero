/*
 * API
 * ---
 * casstkpush
 * casstkpop
 */

#include <zero/asm.h>

#define CAS_LIST_MARK_POS 0
#define CAS_LIST_MARK_BIT (1L << CAS_LIST_MARK_POS)

static __inline__ void
casstkpush(CAS_LIST_ITEM_TYPE *item, CAS_LIST_ITEM_TYPE **head)
{
    CAS_LIST_ITEM_TYPE *first;
    CAS_LIST_ITEM_TYPE *next;
    CAS_LIST_ITEM_TYPE *orig;

    item ->prev = NULL;
    do {
        if (!m_cmpsetbit((volatile long *)*head, CAS_LIST_MARK_POS)) {
            orig = *head;
            first = (CAS_LIST_ITEM_TYPE *)((uintptr_t)orig
                                           & ~CAS_LIST_MARK_BIT);
            if (first) {
                if (!m_cmpsetbit((volatile long *)&first->next,
                                 CAS_LIST_MARK_POS)) {
                    first->prev = item;
                    item->next = first;
                    m_cmpclrbit(&first->next, CAS_LIST_MARK_POS);
                    m_syncwrite(*head, item);
                } else {
                    m_cmpclrbit((volatile long *)*head, CAS_LIST_MARK_POS);

                    continue;
                }
            } else {
                item->next = NULL;
                m_syncwrite(*head, item);
            }
        }
    } while (1);
}

static __inline__ CAS_LIST_ITEM_TYPE *
casstkpop(CAS_LIST_TYPE **head)
{
    long                 nspin = 8;
    CAS_LIST_ITEM_TYPE  *item = NULL;
    CAS_LIST_ITEM_TYPE **mark = NULL;
    CAS_LIST_ITEM_TYPE  *next;
    CAS_LIST_ITEM_TYPE  *tmp;

    do {
        if (!m_cmpsetbit((volatile long *)(*head), CAS_LIST_MARK_POS)) {
            next = (CAS_LIST_ITEM_TYPE *)((uintptr_t)head & ~CAS_MARK_BIT);
            if (next) {
                if (!m_cmpsetbit((volatile long *)&next->next,
                                     CAS_LIST_MARK_POS)) {
                    tmp = (CAS_LIST_ITEM_TYPE *)((uintptr_t)next->next
                                                 & ~CAS_LIST_MARK_BIT);
                    if (next) {
                        next->prev = NULL;
                    }
                    m_cmpclrbit((volatile long *)&next->next,
                                CAS_LIST_MARK_POS);
                    item = next;
                }
            }
            m_cmpclrbit((volatile long *)(*head), CAS_LIST_MARK_POS);
        }
    } while (!item && (--nspin));

    return item;
}

