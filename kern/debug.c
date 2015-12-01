#include <stdint.h>
#include <zero/types.h>
#include <kern/util.h>

void
kprintregs(void)
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
    kprintf("eax\t0x%lx\n", eax);
    kprintf("ecx\t0x%lx\n", ecx);
    kprintf("edx\t0x%lx\n", edx);
    kprintf("ebx\t0x%lx\n", ebx);
    kprintf("esp\t0x%lx\n", esp);
    kprintf("ebp\t0x%lx\n", ebp);
    kprintf("esi\t0x%lx\n", esi);
    kprintf("edi\t0x%lx\n", edi);

    return;
}

void
kprintgenregs(struct m_genregs *genregs)
{
    kprintf("general-purpose registers\n");
    kprintf("-------------------------\n");
    kprintf("eax\t0x%lx\n", genregs-> eax);
    kprintf("ecx\t0x%lx\n", genregs-> ecx);
    kprintf("edx\t0x%lx\n", genregs-> edx);
    kprintf("ebx\t0x%lx\n", genregs-> ebx);
    kprintf("esp\t0x%lx\n", genregs-> esp);
    kprintf("ebp\t0x%lx\n", genregs-> ebp);
    kprintf("esi\t0x%lx\n", genregs-> esi);
    kprintf("edi\t0x%lx\n", genregs-> edi);

    return;
}

void
kprintsegregs(struct m_segregs *segregs)
{
    kprintf("segment registers\n");
    kprintf("-----------------\n");
    kprintf("ds\t0x%lx\n", segregs->ds);
    kprintf("es\t0x%lx\n", segregs->es);
    kprintf("fs\t0x%lx\n", segregs->fs);
    kprintf("gs\t0x%lx\n", segregs->gs);
}

void
kprintjmpframe(struct m_jmpframe *jmpframe, long havestk)
{
    kprintf("jump/stack frame\n");
    kprintf("----------------\n");
    kprintf("eip\t0x%lx\n", jmpframe->eip);
    kprintf("cs\t%hx\n", jmpframe->cs);
    kprintf("eflags\t0x%lx\n", jmpframe->eflags);
    if (havestk) {
        kprintf("uesp\t0x%lx\n", jmpframe->uesp);
        kprintf("uss\t%hx\n", jmpframe->uss);
    }
}

void
kprinttcb(struct m_tcb *m_tcb, long flg)
{
    kprintf("flg\t%lx\n", m_tcb->flg);
    kprintf("pdbr\t%lx\n", m_tcb->pdbr);
    kprintgenregs(&m_tcb->segregs);
    kprintsegregs(&m_tcb->segregs);
    kprintjmpframe(&m_tcb.iret);

    return;
}

