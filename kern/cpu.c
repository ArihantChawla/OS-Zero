#include <kern/conf.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <kern/cpu.h>
#include <kern/proc/task.h>

extern void         cpuprintinfo(void);

volatile struct cpu k_cputab[NCPU] ALIGNED(PAGESIZE);
extern uint8_t      kernusrstktab[NCPU * KERNSTKSIZE];

void
cpuinit(long unit)
{
    volatile struct cpu       *cpu = &k_cputab[unit];
    volatile struct m_cpuinfo *info = &cpu->info;

    taskinittls(unit, 0);
    cpuprobe(unit, info, &info->cache);
    cpu->flg |= CPUINITBIT | CPUHASINFO;

    return;
};

