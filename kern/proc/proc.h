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
#include <kern/mem.h>
#include <kern/proc/task.h>
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

/* descriptor table size */
#define NDESCTAB     (1 << NDESCTABLOG2)
#define NDESCTABLOG2 10
#define NVMHDRTAB    (NPAGEMAX >> PAGESIZELOG2)

#endif /* __KERN_PROC_H__ */

