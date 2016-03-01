#ifndef __ZERO_CASQUEUE_H__
#define __ZERO_CASQUEUE_H__

/*
 * API
 * ---
 * casqueuepush
 * casqueuepop
 */

#include <stddef.h>
#include <zero/asm.h>

#define CAS_QUEUE_MARK_POS 0
#define CAS_QUEUE_MARK_BIT (1L << CAS_QUEUE_MARK_POS)

static __inline__ void
casqueuepush(CAS_QUEUE_ITEM_TYPE *item, CAS_QUEUE_ITEM_TYPE *tab)
{
    CAS_QUEUE_ITEM_TYPE *next;
    
    item ->prev = NULL;
    do {
        if (!m_cmpsetbit((volatile long *)&tab->head,
                         CAS_QUEUE_MARK_POS)) {
            next = (struct mag *)((uintptr_t)tab->head
                                  & ~CAS_QUEUE_MARK_BIT);
            if (first) {
                item->next = next;
                next->prev = item;
                m_syncwrite((volatile long *)&tab->head, item);
                    
                    return;
            } else if (!m_cmpsetbit((volatile long *)&tab->tail,
                                    CAS_QUEUE_MARK_POS)) {
                m_syncwrite((volatile long *)&tab->tail, item);
                m_syncwrite((volatile long *)&tab->head, item);
                
                return;
            } else {
                m_cmpclrbit((volatile long *)&tab->head,
                            CAS_QUEUE_MARK_POS);
                
                continue;
            }
        } else {

            continue;
        }
    } while (1);
}

static __inline__ void
casqueuepushmany(CAS_QUEUE_ITEM_TYPE *first,
                 CAS_QUEUE_ITEM_TYPE *last,
                 CAS_QUEUE_ITEM_TYPE *tab)
{
    CAS_QUEUE_ITEM_TYPE *next;
    
    first->prev = NULL;
    do {
        if (!m_cmpsetbit((volatile long *)&tab->head,
                         CAS_QUEUE_MARK_POS)) {
            next = (struct mag *)((uintptr_t)(tab->head
                                              & ~CAS_QUEUE_MARK_BIT));
            if (next) {
                last->next = next;
                next->prev = last;
                m_syncwrite(*(head), first);

                return;
            } else if (!m_cmpsetbit((volatile long *)&tab->tail,
                                    CAS_QUEUE_MARK_POS)) {
                m_syncwrite((volatile long *)&tab->tail, last);
                m_syncwrite((volatile long *)&tab->head, first);
                
                return;
            } else {
                m_cmpclrbit((volatile long *)&tab->head,
                            CAS_QUEUE_MARK_POS);
                
                continue;
            }
        } else {

            continue;
        }
    } while (1);
}

static __inline__ CAS_QUEUE_ITEM_TYPE *
casqueuepop(CAS_QUEUE_TYPE *tab)
{
    long                 nspin = 8;
    CAS_QUEUE_ITEM_TYPE *item = NULL;
    CAS_QUEUE_ITEM_TYPE *head;
    
    do {
        if (!m_cmpsetbit((volatile long *)&tab->head, CAS_QUEUE_MARK_POS)) {
            head = (struct mag *)((uintptr_t)tab->head
                                  & ~CAS_QUEUE_MARK_BIT);
            if (head) {
                if (head->next) {
                }
                m_syncwrite((volatile long *)&tab->head, next->next);
            }
        }
    } while (!item && (--nspin));
    
    return item;
}

static __inline__ void
casqueuepushtail(CAS_QUEUE_ITEM_TYPE *item,
                 CAS_QUEUE_ITEM_TYPE **tail,
                 CAS_QUEUE_ITEM_TYPE *tab)
{
    struct mag *mark;
    
    mark = (struct mag *)((uintptr_t)tail | CAS_QUEUE_MARK_BIT);
    do {
        if (!m_cmpsetbit((volatile long *)&(*(tail))->next,
                         CAS_QUEUE_MARK_POS)) {
            if (!tail->prev) {
                if (!m_cmpsetbit((volatile long *)&(*(head))->next,
                                 CAS_QUEUE_MARK_POS)) {
                    m_atomwrite((volatile long *)&
                    
                } else {
                    m_cmpclrbit((volatile long *)&(*(tail)->next),
                                CAS_QUEUE_MARK_POS);
                    
                    continue;
                }
            }
        }
    } while (1);
}

#endif /* __ZERO_CASQUEUE_H__ */

