#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/cpu.h>

extern void    cpuprintinfo(void);

struct cpu     cputab[NCPU] ALIGNED(PAGESIZE);
extern uint8_t kernusrstktab[NCPU * KERNSTKSIZE];

long
cpuinit(long id)
{
    struct cpu       *cpu = &cputab[id];
    struct m_cpu     *m_cpu = &cpu->m_cpu;
    struct m_cpuinfo *info = &cpu->m_cpu.info;

    m_cpu->unit = id;
    m_cpu->cpu = cpu;
    cpuprobe(info);
    cpuprintinfo();

    return id;
};



