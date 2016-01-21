#include <kern/conf.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <kern/cpu.h>

struct cpu cputab[NCPU] ALIGNED(PAGESIZE);

