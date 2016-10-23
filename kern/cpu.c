#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/cpu.h>

extern void    taskinittls(long unit);
extern void    cpuprintinfo(void);

struct cpu     cputab[NCPU] ALIGNED(PAGESIZE);
extern uint8_t kernusrstktab[NCPU * KERNSTKSIZE];

void
cpuinit(long unit)
{
    struct cpu       *cpu = &cputab[unit];
    struct m_cpuinfo *info = &cpu->info;

    taskinittls(unit);
    cpuprobe(info, &info->cache);
    cpu->flg |= CPUINITBIT | CPUHASINFO;

    return;
};


