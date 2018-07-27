#include <kern/conf.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <kern/cpu.h>
#include <kern/proc/task.h>
#include <kern/unit/ia32/task.h>

extern void         cpuprintinfo(void);

volatile struct cpu k_cputab[NCPU] ALIGNED(PAGESIZE);
extern uint8_t      kernusrstktab[NCPU * KERNSTKSIZE];

void
cpuinit(struct cpu *cpu)
{
    volatile struct m_cpuinfo *info = &cpu->info;
    long                       flg;

    cpu->cpu = cpu;
    cpuprobe(cpu);
    cpu->flg |= CPUINITBIT | CPUHASINFO;

    return;
};

