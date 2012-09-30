/*
 * map a keyboard event kernel buffer to new task's virtual address PAGESIZE
 * allocate descriptor table
 * initialise <stdio.h> facilities
 * - stdin, stdout, stderr
 */

#include <kern/proc.h>
#include <kern/task.h>
#include <kern/mem.h>
#if defined(__i386__)
#include <kern/unit/ia32/cpu.h>
#endif

/* see <kern/proc.h> for definitions of scheduler classes */
struct proc *
newproc(int argc, char *argv[], char *envp[], int class)
{
    struct proc *proc = kmalloc(sizeof(struct proc));

    proc->state = PROCINIT;
    proc->class = class;
    proc->pid = taskgetpid();
    proc->parent = curproc->pid;

    return proc;
}

