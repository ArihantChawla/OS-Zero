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
    kprintf("eax\t%lx\n", eax);
    kprintf("ecx\t%lx\n", ecx);
    kprintf("edx\t%lx\n", edx);
    kprintf("ebx\t%lx\n", ebx);
    kprintf("esp\t%lx\n", esp);
    kprintf("ebp\t%lx\n", ebp);
    kprintf("esi\t%lx\n", esi);
    kprintf("edi\t%lx\n", edi);

    return;
}

void
kprintgenregs(struct m_genregs *genregs)
{
    kprintf("general-purpose registers\n");
    kprintf("-------------------------\n");
    kprintf("eax\t%lx\n", genregs-> eax);
    kprintf("ecx\t%lx\n", genregs-> ecx);
    kprintf("edx\t%lx\n", genregs-> edx);
    kprintf("ebx\t%lx\n", genregs-> ebx);
    kprintf("esp\t%lx\n", genregs-> esp);
    kprintf("ebp\t%lx\n", genregs-> ebp);
    kprintf("esi\t%lx\n", genregs-> esi);
    kprintf("edi\t%lx\n", genregs-> edi);

    return;
}

void
kprintsegregs(struct m_segregs *segregs)
{
    kprintf("segment registers\n");
    kprintf("-----------------\n");
    kprintf("ds\t%lx\n", segregs->ds);
    kprintf("es\t%lx\n", segregs->es);
    kprintf("fs\t%lx\n", segregs->fs);
    kprintf("gs\t%lx\n", segregs->gs);
}

void
kprintjmpframe(struct m_jmpframe *jmpframe, long havestk)
{
    kprintf("jump/stack frame\n");
    kprintf("----------------\n");
    kprintf("eip\t%lx\n", jmpframe->eip);
    kprintf("cs\t%hx\n", jmpframe->cs);
    kprintf("eflags\t%lx\n", jmpframe->eflags);
    if (havestk) {
        kprintf("uesp\t%lx\n", jmpframe->uesp);
        kprintf("uss\t%hx\n", jmpframe->uss);
    }
}

void
kprinttcb(struct m_tcb *m_tcb, long flg)
{
    
}

