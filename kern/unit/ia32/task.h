#ifndef __KERN_UNIT_IA32_TASK_H__
#define __KERN_UNIT_IA32_TASK_H__

#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>

#if 0
ASMLINK void  m_tcbsave(struct m_tcb *tcb);
#endif
FASTCALL void m_tcbjmp(struct m_tcb *tcb);

#endif /* __KERN_UNIT_IA32_TASK_H__ */

