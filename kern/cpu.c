#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/cpu.h>

extern void    cpuprintinfo(void);

struct m_cpu   m_cputab[NCPU] ALIGNED(PAGESIZE);
extern uint8_t kernusrstktab[NCPU * KERNSTKSIZE];

long
cpuinit(long id)
{
    struct m_cpu     *m_cpu = &m_cputab[id];
    struct m_cpuinfo *info = &m_cpu->info;

    k_curcpu = m_cpu;
    cpuprobe(info, &info->cache);
    m_cpu->data.flg |= CPUINITBIT;

    return id;
};



