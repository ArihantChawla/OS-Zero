#include <stdint.h>
#include <mach/types.h>
#include <kern/util.h>
#include <kern/printf.h>
#include <kern/debug.h>

void
m_printregs32(void)
{
    int32_t eax;
    int32_t ecx;
    int32_t edx;
    int32_t ebx;
    int32_t esp;
    int32_t ebp;
    int32_t esi;
    int32_t edi;

    __asm__ __volatile__ ("movl %%eax, %0\n"
                          "movl %%ecx, %1\n"
                          "movl %%edx, %2\n"
                          "movl %%ebx, %3\n"
                          "movl %%esp, %4\n"
                          "movl %%ebp, %5\n"
                          "movl %%esi, %6\n"
                          "movl %%edi, %7\n"
                          : "=m" (eax), "=m" (ecx), "=m" (edx), "=m" (ebx),
                            "=m" (esp), "=m" (ebp), "=m" (esi), "=m" (edi));
    kprintf("general-purpose registers\n");
    kprintf("-------------------------\n");
    kprintf("eax\t0x%lx\n", (long)eax);
    kprintf("ecx\t0x%lx\n", (long)ecx);
    kprintf("edx\t0x%lx\n", (long)edx);
    kprintf("ebx\t0x%lx\n", (long)ebx);
    kprintf("esp\t0x%lx\n", (long)esp);
    kprintf("ebp\t0x%lx\n", (long)ebp);
    kprintf("esi\t0x%lx\n", (long)esi);
    kprintf("edi\t0x%lx\n", (long)edi);

    return;
}

void
m_printgenregs(struct m_genregs *genregs)
{
    kprintf("general-purpose registers\n");
    kprintf("-------------------------\n");
    kprintf("eax\t0x%lx\n", (long)genregs-> eax);
    kprintf("ecx\t0x%lx\n", (long)genregs-> ecx);
    kprintf("edx\t0x%lx\n", (long)genregs-> edx);
    kprintf("ebx\t0x%lx\n", (long)genregs-> ebx);
    kprintf("esp\t0x%lx\n", (long)genregs-> esp);
    kprintf("ebp\t0x%lx\n", (long)genregs-> ebp);
    kprintf("esi\t0x%lx\n", (long)genregs-> esi);
    kprintf("edi\t0x%lx\n", (long)genregs-> edi);

    return;
}

void
m_printsegregs(struct m_segregs *segregs)
{
    kprintf("segment registers\n");
    kprintf("-----------------\n");
    kprintf("ds\t0x%lx\n", (long)segregs->ds);
    kprintf("es\t0x%lx\n", (long)segregs->es);
    kprintf("fs\t0x%lx\n", (long)segregs->fs);
    kprintf("gs\t0x%lx\n", (long)segregs->gs);
}

void
m_printtrapframe(struct m_trapframe *trapframe, long havestk)
{
    kprintf("jump/stack frame\n");
    kprintf("----------------\n");
    kprintf("eip\t0x%lx\n", (long)trapframe->eip);
    kprintf("cs\t%hx\n", trapframe->cs);
    kprintf("eflags\t0x%lx\n", (long)trapframe->eflags);
    if (havestk) {
        kprintf("uesp\t0x%lx\n", (long)trapframe->uesp);
        kprintf("uss\t%hx\n", trapframe->uss);
    }

    return;
}

void
m_printtask(struct m_task *task, long flg)
{
    kprintf("flg\t%lx\n", (long)task->flg);
    kprintf("pdbr\t%lx\n", (long)task->tcb.pdbr);
    m_printgenregs(&task->tcb.genregs);
    m_printsegregs(&task->tcb.segregs);
    m_printtrapframe(&task->tcb.trapframe, flg & M_TRAPFRAMESTK);

    return;
}

