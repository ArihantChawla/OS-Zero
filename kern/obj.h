#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <stdint.h>
#include <signal.h>
#include <sys/types.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/types.h>
#include <kern/types.h>
#include <kern/syscall.h>
#include <kern/proc/thr.h>

/* descriptor table size */
#define OBJNDESC     (1 << OBJNDESCLOG2)
#define OBJNDESCLOG2 16

/* user + group credentials */
struct cred {
    uid_t uid;                          // user ID
    gid_t gid;                          // group ID
} PACK();

/* permissions */
struct perm {
    struct cred owner;                  // user + group
    long        flg;                    // permission bits
};

/* thread */
/* states */
#define THRNONE   0x00                 // undefined
#define THRINIT   0x01                 // being initialised
#define THRRUN    0x02                 // running
#define THRREADY  0x03                 // ready to run
#define THRWAIT   0x04                 // waiting on system descriptor
#define THRSTOP   0x05                 // stopped
#define THRZOMBIE 0x06                 // finished but not waited for
struct thr {
    /* thread control block */
    struct m_tcb   m_tcb;               // context
    /* thread stacks */
    uintptr_t      ustk;                // user-mode stack
    uintptr_t      kstk;                // kernel-mode stack
    /* state */
    long           state;               // thread state
    /* wait channel */
    uintptr_t      wchan;               // wait channel
    /* linkage */
    struct proc   *proc;                // owner process
    /* queue linkage */
    struct thr    *prev;                // previous in queue
    struct thr    *next;                // next in queue
    long           id;
    /* scheduler parameters */
    long           nice;                // priority adjustment
    long           class;               // thread class
    long           prio;                // priority
//    long           interact;
    long           runtime;             // run time
    /* system call context */
    struct syscall syscall;
} PACK();

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
    desc_t           *dtab2;
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
//    struct ringbuf   evbuf;
} PACK() ALIGNED(PAGESIZE);

/* memory region */
struct memreg {
    struct perm   perm;                 // memory permissions
    unsigned long flg;                  // flags
    uintptr_t     base;                 // base address
    unsigned long size;                 // size in bytes
    off_t         ofs;                  // region offset
};

/* I/O node */
struct node {
    struct perm   perm; 		// node permissions
    unsigned long type; 		// file, dir, pipe, mq, shm, sock
    desc_t        desc; 		// system descriptor
    unsigned long flg;  		// NODESEEKBIT, NODEMAPBIT, etc.
    unsigned long nref; 		// reference count
    off_t         pos;                  // seek position
    /* TODO: interface function pointers */
};

#endif /* __KERN_OBJ_H__ */

