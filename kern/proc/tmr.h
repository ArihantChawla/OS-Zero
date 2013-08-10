#ifndef __KERN_PROC_TMR_H__
#define __KERN_PROC_TMR_H__

void tmrinit(void);
long tmrget(long id);
long tmrset(long id, long tid, long nnsec);

#endif /* __KERN_PROC_TMR_H__ */

