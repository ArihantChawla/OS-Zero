#ifndef __KERN_PROC_H__
#define __KERN_PROC_H__

/*
 * process startup
 * ---------------
 *
 * kernel world
 * ------------
 * - initialise segmentation
 * - initialise interrupt management
 * - initialise memory management
 * - acquire and initialise process and thread structures
 * - initialize [framebuffer] graphics
 * - initialize event interface
 * - initialize page daemon
 * - initialize [block I/O] buffer management
 * - execute init
 *
 * init
 * ----
 * - system startup
 *   - services; httpd, sshd, etc.
 *
 * user world
 * ----------
 * - allocate descriptor table
 *  - initialise <stdio.h> facilities
 * - stdin, stdout, stderr
 *   - keyboard and console drivers
 * - argc, argv, envp
 * - set up for crt0
 *   - initialise process structure
 *   - acquire process and thread IDs
 *   - define _text, _bss, _ebss, _start
 *   - set up user and system (ring 0) stacks at zero (the highest part of virtual address space)
 *   - map kernel segment at 3 gigabytes (32-bit systems) or -2 GB
 *   - load, link, and map program segments
 *   - configure default signal handlers
 *
 * userland (c standard library)
 * -----------------------------
 * - crt0
 *   - define _start to start of loaded .text section
 *   - program initialisation
 *     - load and runtime-link image
 *   - zero .bss section
 *   - call main()
 *   - call exit()
 */

#define __KERNEL__ 1
#include <stddef.h>
#include <limits.h>
#include <signal.h>
#include <stdint.h>
#include <sys/types.h>
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

/* system-assigned process IDs */
#define PROCKERN     0 // main kernel; context switches, system calls, ...
#define PROCINIT     1 // init process; the mother of all processes
#define PROCEVD      2 // event daemon; receive and dispatch events
#define PROCPAGED    3 // page daemon; page replacement; aging, LRU-queue
#define PROCBUFD     4 // buffer daemon; flush dirty buffers to disk
#define PROCIDLE     5 // idle process; zeroes memory etc.
#define PROCNPREDEF  6

long procinit(long id);
long procgetpid(void);
void procfreepid(long id);

/* process states */
#define TASK_CREATE  0
#define TASK_USER    1
#define TASK_KERNEL  2
#define TASK_READY   3
#define TASK_WAIT    4
#define TASK_STOP    5
#define TASK_ZOMBIE  6
#define TASK_NSTATE  7


/* descriptor table size */
#define OBJNDESC     (1 << OBJNDESCLOG2)
#define OBJNDESCLOG2 20

/* process */
struct proc {
    long              nice;
    long              sched;
    long              prio;
    struct thr       *thr;              // current running thread
    long              nthr;             // # of threads
    struct thr      **thrtab;           // child threads
    /* round-robin queue */
//    struct thrq       thrq;             // queue of ready threads
    /* page directory */
    pde_t            *pdir;             // page directory address
    /* stacks */
    uint8_t          *ustk;             // user-mode stack
    uint8_t          *kstk;             // kernel-mode stack
    /* memory attributes */
    uint8_t          *brk;              // current heap-top
    /* process credentials */
    pid_t             pid;              // process ID
    pid_t             parent;           // parent process
    uid_t             ruid;             // real user ID
    gid_t             rgid;             // real group ID
    uid_t             euid;             // effective user ID
    gid_t             egid;             // effective group ID
    /* current permission mask */
    mode_t            umask;
    /* descriptor tables */
    size_t            ndtab;		// number of entries in descriptor table
    uintptr_t        *dtab;		// descriptor table
    /* signal state */
    sigset_t          sigmask;          // signal mask
    sigset_t          sigpend;          // pending signals
    signalhandler_t  *sigvec[NSIG];
    /* current working directory */
    char             *cwd;
    /* runtime arguments */
    long              argc;             // argument count
    char            **argv;             // argument vector
    char            **envp;             // environment strings
    /* memory management */
    struct slabhdr   *vmtab[PTRBITS];
    /* keyboard input buffer */
    void             *kbdbuf;
#if 0
    /* event queue */
    struct ev        *evq;
#endif
} ALIGNED(PAGESIZE);

#endif /* __KERN_PROC_H__ */

