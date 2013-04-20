#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/mtx.h>
#include <kern/sched.h>

void (*schedyield)(void);

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

