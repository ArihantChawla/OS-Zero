#ifndef __KERN_PROC_PROC_H__
#define __KERN_PROC_PROC_H__

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
#include <kern/cred.h>
#include <kern/syscall.h>
#include <kern/proc/task.h>
#include <kern/unit/x86/cpu.h>
#include <kern/unit/x86/vm.h>

/* system-assigned process IDs */
#define PROCKERN     0 // main kernel; context switches, system calls, ...
#define PROCINIT     1 // init process; the mother of all processes
#define PROCEVD      2 // event daemon; receive and dispatch events
#define PROCPAGED    3 // page daemon; page replacement; aging, LRU-queue
#define PROCBUFD     4 // buffer daemon; flush dirty buffers to disk
#define PROCIDLE     5 // idle process; zeroes memory etc.
#define TASKNPREDEF  6

long procinit(long id);
long procgetpid(void);
void procfreepid(long id);

/* process segments other than the stacks */
struct procseginfo {
    void   *trapvec;
    size_t  trapvecsize;
    void   *text;
    size_t  textsize;
    void   *rodata;
    size_t  rodatasize;
    void   *data;
    size_t  datasize;
    void   *bss;
    size_t  heapsize;
    void   *brk;
};

/* process structure */
/*
 * termination status (status-member)
 * ----------------------------------
 * - exit status byte in low 8 bits
 * - termination signal in second-lowest 8 bits
 * - stop signal in third-lowest 8 bits
 * - flags in high bits
 */
#define procexitstatus(proc) ((proc)->status & 0xff)
#define procexitsig(proc)    (((proc)->status >> 8) & 0xff)
#define procstopsig(proc)    (((proc)->status >> 16) & 0xff)
#define PROCEXITED           (1 << 31)
#define PROCSIGNALED         (1 << 30)
struct proc {
    /* process and thread management */
    struct task         *task;
    struct task         *thrqueue;
    long                 nthr;          // # of child threads
    struct task        **thrtab;        // child threads
    /* memory management */
    struct vmpagemap     vmpagemap;
    uint8_t             *brk;           // current heap-top
    struct procseginfo  *seginfo;       // process segment information
    /* WORDSIZE * CHAR_BIT LRU-queues */
    struct physpage    **pagelru;       // in-core physical pages
    struct slabhdr     **vmslabtab;     // PTRBITS queues of slabs
    /* process credentials */
    struct cred         *cred;          // effective credentials
    struct cred         *realcred;      // real credentials
    struct cred         *savecred;      // saved credentials
    /* descriptor tables */
    size_t               ndesctab;      // number of entries in descriptor table
    struct desc         *desctab;       // descriptor table
    /* current working directory */
    char                *cwd;
    /* current permission mask */
    mode_t               umask;
    /* signal dispositions */
    signalhandler_t     *sigvec[NSIG];
    /* runtime arguments */
    long                 argc;          // argument count
    char               **argv;          // argument vector
    char               **envp;          // environment strings
    /* keyboard input buffer */
    void                *kbdbuf;
    long                 status;        // termination status
#if 0
    /* event queue */
    struct ev           *evq;
#endif
} ALIGNED(PAGESIZE);

#endif /* __KERN_PROC_PROC_H__ */

