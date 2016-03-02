#ifndef __ZERO_CASQUEUE_H__
#define __ZERO_CASQUEUE_H__

/*
 * API
 * ---
 * casqueuepush
 * casqueuepop
 */

#include <stddef.h>
#include <stdio.h>
#include <zero/asm.h>

#define CAS_QUEUE_MARK_POS 0
#define CAS_QUEUE_MARK_BIT (1L << CAS_QUEUE_MARK_POS)

static __inline__ void
casqueuepush(CAS_QUEUE_ITEM_TYPE *item, CAS_QUEUE_TYPE *queue)
{
    CAS_QUEUE_ITEM_TYPE *head;

    item->prev = NULL;
    do {
        if (!m_cmpsetbit((volatile long *)&queue->ptr,
                         CAS_QUEUE_MARK_POS)) {
            if (!m_cmpsetbit((volatile long *)&queue->tail,
                             CAS_QUEUE_MARK_POS)) {
                head = (CAS_QUEUE_ITEM_TYPE *)((uintptr_t)queue->ptr
                                               & ~CAS_QUEUE_MARK_BIT);
                if (head) {
                    if (!m_cmpsetbit((volatile long *)&head->next,
                                     CAS_QUEUE_MARK_POS)) {
                        head->prev = item;
                        m_cmpclrbit((volatile long *)&head->next,
                                    CAS_QUEUE_MARK_POS);
                        m_cmpclrbit((volatile long *)&queue->tail,
                                    CAS_QUEUE_MARK_POS);
                    } else {
                        m_cmpclrbit((volatile long *)&queue->tail,
                                    CAS_QUEUE_MARK_POS);
                        m_cmpclrbit((volatile long *)&queue->ptr,
                                    CAS_QUEUE_MARK_POS);
                        
                        continue;
                    }
                } else {
                    m_syncwrite((volatile long *)&queue->tail, item);
                }
            }
            m_syncwrite((volatile long *)&queue->ptr, item);
            
            return;
        } else {

            continue;
        }
    } while (1);
}

static __inline__ void
casqueuepushmany(CAS_QUEUE_ITEM_TYPE *first,
                 CAS_QUEUE_ITEM_TYPE *last,
                 CAS_QUEUE_TYPE *queue)
{
    CAS_QUEUE_ITEM_TYPE *head;
    
    first->prev = NULL;
    do {
        if (!m_cmpsetbit((volatile long *)&queue->ptr,
                         CAS_QUEUE_MARK_POS)) {
            if (!m_cmpsetbit((volatile long *)&queue->tail,
                             CAS_QUEUE_MARK_POS)) {
                head = ((CAS_QUEUE_ITEM_TYPE *)
                        ((uintptr_t)queue->ptr & ~CAS_QUEUE_MARK_BIT));
                if (head) {
                    if (!m_cmpsetbit((volatile long *)&head->next,
                                     CAS_QUEUE_MARK_POS)) {
                        last->next = head;
                        head->prev = last;
                        m_cmpclrbit((volatile long *)&head->next,
                                    CAS_QUEUE_MARK_POS);
                    } else {
                        m_cmpclrbit((volatile long *)&queue->tail,
                                    CAS_QUEUE_MARK_POS);
                        m_cmpclrbit((volatile long *)&queue->ptr,
                                    CAS_QUEUE_MARK_POS);
                        
                        continue;
                    }
                }                
                m_syncwrite((volatile long *)&queue->tail, last);
            } else {
                m_cmpclrbit((volatile long *)&queue->ptr,
                            CAS_QUEUE_MARK_POS);
                
                continue;
            }
            m_syncwrite((volatile long *)&queue->ptr, first);
            
            return;
        } else {
            
            continue;
        }
    } while (1);
}

static __inline__ CAS_QUEUE_ITEM_TYPE *
casqueuepop(CAS_QUEUE_TYPE *queue)
{
    long                 nspin = 8;
    CAS_QUEUE_ITEM_TYPE *item;
    CAS_QUEUE_ITEM_TYPE *head;
    CAS_QUEUE_ITEM_TYPE *next;
    
    do {
        item = NULL;
        if (!m_cmpsetbit((volatile long *)&queue->ptr,
                         CAS_QUEUE_MARK_POS)) {
            if (!m_cmpsetbit((volatile long *)&queue->tail,
                             CAS_QUEUE_MARK_POS)) {
                head = (CAS_QUEUE_ITEM_TYPE *)((uintptr_t)queue->ptr
                                               & ~CAS_QUEUE_MARK_BIT);
                if (head) {
                    if (!m_cmpsetbit((volatile long *)&head->next,
                                     CAS_QUEUE_MARK_POS)) {
                        item = head;
                        next = (CAS_QUEUE_ITEM_TYPE *)((uintptr_t)head->next
                                                       & ~CAS_QUEUE_MARK_BIT);
                        if (next) {
                            if (!m_cmpsetbit((volatile long *)&next->next,
                                             CAS_QUEUE_MARK_POS)) {
                                next->prev = NULL;
                                m_cmpclrbit((volatile long *)&next->next,
                                            CAS_QUEUE_MARK_POS);
                            } else {
                                m_cmpclrbit((volatile long *)&head->next,
                                            CAS_QUEUE_MARK_POS);
                                m_cmpclrbit((volatile long *)&queue->ptr,
                                            CAS_QUEUE_MARK_POS);
                                nspin--;
                                if (!nspin) {
                                    
                                    return NULL;
                                }                                
                                
                                continue;
                            }
                        }
                        m_syncwrite((volatile long *)&queue->ptr, head->next);
                    } else {
                        m_cmpclrbit((volatile long *)&head->next,
                                    CAS_QUEUE_MARK_POS);
                        m_cmpclrbit((volatile long *)&queue->ptr,
                                    CAS_QUEUE_MARK_POS);
                        nspin--;
                        if (!nspin) {
                            
                            return NULL;
                        }                                
                    }
                    m_cmpclrbit((volatile long *)&head->next,
                                CAS_QUEUE_MARK_POS);
                }
                m_cmpclrbit((volatile long *)&queue->tail,
                            CAS_QUEUE_MARK_POS);
            }
            m_cmpclrbit((volatile long *)&queue->ptr,
                        CAS_QUEUE_MARK_POS);
        }
    } while (!item &&(--nspin));
    
    return item;
}

#if 0

static __inline__ void
casqueuepushtail(CAS_QUEUE_ITEM_TYPE *item,
                 CAS_QUEUE_TYPE *queue)
{
    CAS_QUEUE_ITEM_TYPE *tail;
    
    item ->next = NULL;
    do {
        if (!m_cmpsetbit((volatile long *)&queue->ptr,
                         CAS_QUEUE_MARK_POS)) {
            if (!m_cmpsetbit((volatile long *)&queue->tail,
                             CAS_QUEUE_MARK_POS)) {
                item->next = (CAS_QUEUE_ITEM_TYPE *)CAS_QUEUE_MARK_BIT;
                tail = (CAS_QUEUE_ITEM_TYPE *)((uintptr_t)queue->tail
                                               & ~CAS_QUEUE_MARK_BIT);
                if (tail) {
                    if (!m_cmpsetbit((volatile long *)&tail->next,
                                     CAS_QUEUE_MARK_POS)) {
                        item->prev = tail;
                        m_syncwrite((volatile long *)&tail->next, item);
                    } else {
                        m_cmpclrbit((volatile long *)&queue->tail,
                                    CAS_QUEUE_MARK_POS);
                        
                        continue;
                    }
                } else {
                    item->prev = NULL;
                    m_syncwrite((volatile long *)&queue->ptr, item);
                }
            } else {
                m_cmpclrbit((volatile long *)&queue->ptr,
                            CAS_QUEUE_MARK_POS);
                
                continue;
            }
            m_syncwrite((volatile long *)&queue->tail, item);
            m_cmpclrbit((volatile long *)&queue->ptr,
                        CAS_QUEUE_MARK_POS);
            
            return;
        } else {

            continue;
        }
    } while (1);
}

#endif

#endif /* __ZERO_CASQUEUE_H__ */

