#ifndef __PROC_H__
#define __PROC_H__

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
#if defined(__i386__)
#include <kern/unit/ia32/cpu.h>
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
struct proc {
    struct thr      *thr;               // current thread
    /* wait channel */
    void            *wchan;             // wait channel
    /* page directory */
    pde_t           *pdir;
    /* kernel stack */
    uint8_t         *kstk;               // kernel-mode stack (wired)
    long             state;
    long             class;
    /* memory attributes */
    uint8_t         *brk;
    /* process credentials */
    pid_t            pid;                // process ID
    pid_t            parent;             // parent process
    long             nthr;               // # of threads
    uid_t            ruid;               // real user ID
    gid_t            rgid;               // real group ID
    uid_t            euid;               // effective user ID
    gid_t            egid;               // effective group ID
    /* descriptors */
    uintptr_t       *dtab;
    uintptr_t       *dtab2;
    /* signal state */
    sigset_t         sigmask;
    sigset_t         sigpend;
    signalhandler_t *sigvec[NSIG];
    /* system call context */
    struct syscall   syscall;
    struct slabhdr  *vmtab[PTRBITS];
    /* event queue */
//    struct ringbuf   evbuf;
} PACK() ALIGNED(PAGESIZE);

#endif /* __PROC_H__ */

