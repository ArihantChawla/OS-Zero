#ifndef __KERN_UNIT_IA32_THR_H__
#define __KERN_UNIT_IA32_THR_H__

#if 0
static __inline__ void
m_tcbsave(struct m_tcb *tcb)
{
    __asm__ __volatile__ ("movl %0, %%esp\n"
                          :
                          : "m" (tcb));
    /* save FPU state */
    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxsave (%esp)\n");
    } else {
        __asm__ __volatile__ ("fnsave (%esp)\n");
    }
    /* push segment registers */
    __asm__ __volatile__ ("leal %c0(%%esp), %%esp\n"
                          :
                          : "i" (offsetof(struct m_tcb, segregs)
                                 + sizeof(struct m_segregs)));
    __asm__ __volatile__ ("pushl %gs\n");
    __asm__ __volatile__ ("pushl %fs\n");
    __asm__ __volatile__ ("pushl %es\n");
    __asm__ __volatile__ ("pushl %ds\n");
    /* push general-purpose registers */
    __asm__ __volatile__ ("pushal\n");
    /* push PDBR */
    __asm__ __volatile__ ("movl %cr3, %eax\n");
    __asm__ __volatile__ ("pushl %eax\n");
    /* construct iret return frame */
    __asm__ __volatile__ ("pushl %ss\n");
    __asm__ __volatile__ ("pushl %esp\n");
    __asm__ __volatile__ ("pushfl\n");
    __asm__ __volatile__ ("pushl %cs\n");
    __asm__ __volatile__ ("movl 4(%ebp), %eax\n");
    __asm__ __volatile__ ("pushl %eax\n");
    __asm__ __volatile__ ("movl %ebp, %esp\n");

    return;
}

static __inline__ void
m_tcbjmp(struct m_tcb *tcb)
{
    __asm__ __volatile__ ("movl %0, %%esp\n"
                          :
                          : "m" (tcb));
    /* restore FPU state */
    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxrstor (%esp)\n");
    } else {
        __asm__ __volatile__ ("frstor (%esp)\n");
    }
    __asm__ __volatile__ ("leal %c0(%%esp), %%esp\n"
                          :
                          : "i" (offsetof(struct m_tcb, pdbr)));
    /* restore PDBR */
    __asm__ __volatile__ ("popl %eax\n");
    __asm__ __volatile__ ("movl %eax, %cr3\n");
    /* restore segment registers */
    __asm__ __volatile__ ("popl %ds\n");
    __asm__ __volatile__ ("popl %es\n");
    __asm__ __volatile__ ("popl %fs\n");
    __asm__ __volatile__ ("popl %gs\n");
    /* restore general-purpose registers */
    __asm__ __volatile__ ("popal\n");
    /* jump to thread with IRET */
    __asm__ __volatile__ ("leal %c0(%%esp), %%esp\n"
                          :
                          : "i" (-(sizeof(struct m_trapframe)
                                   + sizeof(int32_t)
                                   + sizeof(struct m_pusha))));
    __asm__ __volatile__ ("iret\n");

    /* NOTREACHED */
}
#endif /* 0 */

static __inline__ void
m_tcbsave(struct m_tcb *tcb)
{
    __asm__ __volatile__ ("movl %0, %%esp\n"
                          :
                          : "m" (tcb));
    /* save FPU state */
    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxsave (%esp)\n");
    } else {
        __asm__ __volatile__ ("fnsave (%esp)\n");
    }
    /* push segment registers */
    __asm__ __volatile__ ("leal %c0(%%esp), %%esp\n"
                          "pushl %%gs\n"
                          "pushl %%fs\n"
                          "pushl %%es\n"
                          "pushl %%ds\n"
                          /* push general-purpose registers */
                          "pushal\n"
                          /* push PDBR */
                          "movl %%cr3, %%eax\n"
                          "pushl %%eax\n"
                          /* construct iret return frame */
                          "pushl %%ss\n"
                          "pushl %%esp\n"
                          "pushfl\n"
                          "pushl %%cs\n"
                          "movl %c1(%%ebp), %%eax\n"
                          "pushl %%eax\n"
                          "movl %%ebp, %%esp\n"
                          :
                          : "i" (offsetof(struct m_tcb, segregs)
                                 + sizeof(struct m_segregs)),
                            "i" (offsetof(struct m_stkframe, pc))
                          : "memory");

    return;
}

static __inline__ void
m_tcbjmp(struct m_tcb *tcb)
{
    __asm__ __volatile__ ("movl %0, %%esp\n"
                          :
                          : "m" (tcb));
    /* restore FPU state */
    if (k_curcpu->info->flags & CPUHASFXSR) {
        __asm__ __volatile__ ("fxrstor (%esp)\n");
    } else {
        __asm__ __volatile__ ("frstor (%esp)\n");
    }
    __asm__ __volatile__ ("leal %c0(%%esp), %%esp\n"
                          /* restore PDBR */
                          "popl %%eax\n"
                          "movl %%eax, %%cr3\n"
                          /* restore segment registers */
                          "popl %%ds\n"
                          "popl %%es\n"
                          "popl %%fs\n"
                          "popl %%gs\n"
                          /* restore general-purpose registers */
                          "popal\n"
                          /* jump to thread with IRET */
                          "leal %c1(%%esp), %%esp\n"
                          "iret\n"
                          :
                          : "i" (offsetof(struct m_tcb, pdbr)),
                            "i" (-(sizeof(struct m_trapframe)
                                   + sizeof(int32_t)
                                   + sizeof(struct m_pusha)))
                          : "memory");
    /* NOTREACHED */
}
      
#endif /* __KERN_UNIT_IA32_THR_H__ */

