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

/* descriptor table size */
#define OBJNDESC     (1 << OBJNDESCLOG2)
#define OBJNDESCLOG2 16

#endif /* __KERN_PROC_H__ */

