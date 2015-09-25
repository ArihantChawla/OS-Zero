#include <limits.h>
#include <kern/proc/proc.h>

void
uatexit(struct proc *proc)
{
    /* close all open descriptors */
    /* reclaim [physical] memory back to the kernel */
}

