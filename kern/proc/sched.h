#ifndef __KERN_PROC_SCHED_H__
#define __KERN_PROC_SCHED_H__

#include <kern/conf.h>
#include <zero/types.h>

#include <kern/proc/task.h>

//extern volatile FASTCALL struct m_tcb * (*schedpicktask)(void);
extern void schedyield(void);
extern void schedinit(void);

#if (ZEROSCHED)
struct taskprioq {
    volatile long  lk;
    struct task   *head;
    struct task   *tail;
};
#endif

#endif /* __KERN_PROC_SCHED_H__ */

