#ifndef __KERN_PROC_H__
#define __KERN_PROC_H__

/*
 * map a keyboard event kernel buffer to new task's virtual address PAGESIZE
 * allocate descriptor table
 * initialise <stdio.h> facilities
 * - stdin, stdout, stderr
 */

#define __KERNEL__ 1
#include <zero/cdecl.h>
#include <zero/param.h>
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

/* process */
struct proc {
    struct thr       *thr;              // current running thread
    long              nthr;             // # of threads
    /* round-robin queue */
    struct thrq       thrq;             // queue of ready threads
    /* page directory */
    pde_t            *pdir;             // page directory address
    /* stacks */
    uint8_t          *ustk;             // user-mode stack
    uint8_t          *kstk;             // kernel-mode stack
    long              class;
    /* memory attributes */
    uint8_t          *brk;
    /* process credentials */
    pid_t             pid;              // process ID
    pid_t             parent;           // parent process
    uid_t             ruid;             // real user ID
    gid_t             rgid;             // real group ID
    uid_t             euid;             // effective user ID
    gid_t             egid;             // effective group ID
    /* descriptor tables */
    desc_t           *dtab;
    /* signal state */
    sigset_t          sigmask;          // signal mask
    sigset_t          sigpend;          // pending signals
    signalhandler_t  *sigvec[NSIG];
    /* runtime arguments */
    long              argc;             // argument count
    char            **argv;             // argument vector
    char            **envp;             // environment strings
    /* memory management */
    struct slabhdr   *vmtab[PTRBITS];
    /* event queue */
    struct ev        *evq;
} PACK() ALIGNED(PAGESIZE);

#endif /* __KERN_PROC_H__ */

