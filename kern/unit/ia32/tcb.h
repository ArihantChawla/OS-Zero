#ifndef __KERN_UNIT_IA32_TCB_H__
#define __KERN_UNIT_IA32_TCB_H__

/* flg-bits for struct m_tcb */
#define M_NOFP   1
#define M_FP387  2
#define M_FPXMM  3
#define M_FPUSED 0x80000000

#endif /* __KERN_UNIT_IA32_TCB_H__ */

