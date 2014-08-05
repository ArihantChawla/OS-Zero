#ifndef __KERN_UNIT_IA32_THR_H__
#define __KERN_UNIT_IA32_THR_H__

static __inline__ void
m_tcbsave(struct m_tcb *tcb)
{
    __asm__ __volatile__ ("movl %0, %%esp\n" : : "m" (tcb));
    /* save FPU state */
    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxsave (%esp)\n");
    } else {
        __asm__ __volatile__ ("fnsave (%esp)\n");
    }
    __asm__ __volatile__ ("leal %c0(%%esp), %%esp\n"
                          /* save general purpose registers */
                          "pushal\n"
                          /* save PDBR */
                          "movl %%cr3, %%eax\n"
                          "pushl %%eax\n"
                          /* save segment registers */
                          "pushl %%gs\n"
                          "pushl %%fs\n"
                          "pushl %%es\n"
                          "pushl %%ds\n"
                          /* construct iret return frame */
                          "pushl %%ss\n"
                          "pushl %%ebp\n"
                          "pushfl\n"
                          "pushl %%cs\n"
                          "movl %c1(%%ebp), %%eax\n"
                          "pushl %%eax\n"
                          :
                          : "i" (offsetof(struct m_tcb, genregs)
                                 + sizeof(struct m_pusha)),
                            "i" (offsetof(struct m_stkframe, pc)));
                            
    return;
}

static __inline__ void
m_tcbjmp(struct m_tcb *tcb)
{
    __asm__ __volatile__ ("movl %0, %%esp\n" : : "m" (tcb));
    /* restore FPU state */
    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxrstor (%esp)\n");
    } else {
        __asm__ __volatile__ ("frstor (%esp)\n");
    }
    __asm__ __volatile__ ("movl %c0, %%eax\n"
                          "addl %%eax, %%esp\n"
                          /* restore PDBR */
                          "popl %%eax\n"
                          "movl %%eax, %%cr3\n"
                          /* restore general-purpose registers */
                          "popal\n"
                          /* restore segment registers */
                          "popl %%ds\n"
                          "popl %%es\n"
                          "popl %%fs\n"
                          "popl %%gs\n"
                          "movl %1, %%esp\n"
                          "addl %c2, %%esp\n"
                          /* jump to thread with IRET */
                          "iret\n"
                          :
                          : "i" (offsetof(struct m_tcb, pdbr)
                                 + sizeof(int32_t)),
                            "m" (tcb),
                            "i" (offsetof(struct m_tcb, iret))
                          : "memory");
    /* NOTREACHED */
}

#endif /* __KERN_UNIT_IA32_THR_H__ */

