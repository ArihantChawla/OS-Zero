#ifndef __KERN_PROC_H__
#define __KERN_PROC_H__

/*
 * map a keyboard event kernel buffer to new task's virtual address PAGESIZE
 * allocate descriptor table
 * initialise <stdio.h> facilities
 * - stdin, stdout, stderr
 */

#define __KERNEL__ 1
#include <zero/types.h>
#include <kern/types.h>
#include <kern/syscall.h>
#include <kern/task.h>
#include <kern/mem.h>
#if !defined(__arm__)
#include <kern/unit/x86/cpu.h>
#endif


long procinit(long id);
long procgetpid(void);
void procfreepid(long id);

#define SCHEDPRIO 0
#define NTHRPRIO  256

/* I/O scheduler operations */
#define IOSEEK    0     // physical seek
#define IOWRITE   1     // buffered or raw write operation
#define IOREAD    2     // buffered or raw read operation
#define IOSETF    3     // set descriptor flags

/* process states */
#define PROCUNUSED  0x00L       // unused
#define PROCINIT    0x01L       // being initialized
#define PROCSWAPPED 0x02L       // swapped
#define PROCSLEEP   0x03L       // sleeping
#define PROCWAIT    0x04L       // waiting
#define PROCREADY   0x04L       // runnable
#define PROCRUN     0x05L       // running
#define PROCZOMBIE  0x06L       // not waited for
/* descriptor table size */
#define NDESCTAB     (1 << NDESCTABLOG2)
#define NDESCTABLOG2 10
#define NVMHDRTAB    (NPAGEMAX >> PAGESIZELOG2)

#endif /* __KERN_PROC_H__ */

