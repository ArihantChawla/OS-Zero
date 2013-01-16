#include <stddef.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/conf.h>
#include <kern/proc.h>
#include <kern/thr.h>
#include <kern/unit/x86/cpu.h>

struct thr     thrtab[NTHR] ALIGNED(PAGESIZE);
struct thr    *runqueuetab[NPRIO];
volatile long  runqueuelktab[NPRIO];
struct thr    *corethrtab[NCPU];

void
thrinit(long id, long prio)
{
    struct thr *thr = &thrtab[id];

    thr->next = runqueuetab[prio];
    if (thr->next) {
        thr->next->prev = thr;
    }
    runqueuetab[prio] = thr;

    return;
}

void
thrsave(struct thr *thr)
{
    uint32_t pc;

    /* threads return to thryield() */
    m_getretadr(pc);
    m_tcbsave(&thr->m_tcb);
    thr->m_tcb.iret.eip = pc;

    return;
}

void
thrjmp(struct thr *thr)
{
    curthr = thr;
    curproc = thr->proc;
    m_tcbjmp(&thr->m_tcb);

    /* NOTREACHED */
}

