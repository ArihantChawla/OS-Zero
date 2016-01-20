#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/cpu.h>

volatile struct cpu cputab[NCPU] ALIGNED(PAGESIZE);

