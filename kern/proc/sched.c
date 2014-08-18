#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/mtx.h>
#include <kern/proc/sched.h>

volatile FASTCALL struct m_tcb *(*schedyield)(void);

void
schedinit(void)
{
#if (ZEROSCHED)
    schedyield = thryield;
#else
#error define supported scheduler such as ZEROSCHED
#endif

    return;
}

