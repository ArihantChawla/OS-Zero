/*
 * API
 * ---
 * caslistpush
 * caslistpop
 * caslistrm
 */

#include <zero/asm.h>

#define CAS_LIST_MARK_POS 0
#define CAS_LIST_MARK_BIT (1L << CAS_LIST_MARK_POS)

static __inline__ CAS_LIST_ITEM_TYPE *
caslistpop(CAS_LIST_TYPE **head)
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
                if (!m_cmpsetbit((volatile long *)&next->next, CAS_LIST_MARK_POS)) {
                    tmp = (CAS_LIST_ITEM_TYPE *)((uintptr_t)next->next
                                                 & ~CAS_LIST_MARK_BIT);
                    if (next) {
                        next->prev = NULL;
                    }
                    mark = &next->next;
                } else {
                    m_cmpclrbit((volatile long )(*head), CAS_LIST_MARK_POS);

                    continue;
                }
            }
            item = next;
            if (mark) {
                m_cmpclrbit((volatile long *)mark, CAS_LIST_MARK_POS);
            }
            m_cmpclrbit((volatile long *)(*head), CAS_LIST_MARK_POS);
        }
    } while (!item && (--nspin));

    return item;
}

