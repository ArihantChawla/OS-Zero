#ifndef __KERN_UNIT_IA32_BITS_ASM_H__
#define __KERN_UNIT_IA32_BITS_ASM_H__

#include <zero/types.h>

#define M_TCB_SEGREGS   (offsetof(struct m_tcb, segregs))
#define M_TCB_GENREGS   (offsetof(struct m_tcb, genregs))
#define M_TCB_PDBR      (offsetof(struct m_tcb, pdbr))
#define M_TCB_TRAPNUM   (offsetof(struct m_tcb, trapnum))
#define M_TCB_ERR       (offsetof(struct m_tcb, err))
#define M_TCB_TRAPFRAME (offsetof(struct m_tcb, trapnum))

#endif /* __KERN_UNIT_IA32_BITS_ASM_H__ */

