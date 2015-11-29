#include <stdint.h>
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

