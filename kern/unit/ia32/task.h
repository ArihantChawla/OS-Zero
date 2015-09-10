#ifndef __KERN_UNIT_IA32_TASK_H__
#define __KERN_UNIT_IA32_TASK_H__

#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>

ASMLINK void  m_tcbsave(struct m_tcb *tcb);
FASTCALL void m_tcbjmp(struct m_tcb *tcb);

#endif /* __KERN_UNIT_IA32_TASK_H__ */

