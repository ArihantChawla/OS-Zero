#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <stdint.h>
#include <signal.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/types.h>
#include <kern/types.h>
#include <kern/syscall.h>
#include <kern/proc/thr.h>

/* user + group credentials */
struct cred {
    uid_t uid;
    gid_t gid;
} PACK();

/* permissions */
struct perm {
    struct cred owner;  // user + group
    long        flg;    // permission bits
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
    struct m_tcb   m_tcb;
    /* state */
    long           state;
    /* wait channel */
    uintptr_t      wchan;               // wait channel
    /* linkage */
    struct proc   *proc;                // owner process
    /* queue linkage */
    struct thr    *prev;                // previous in queue
    struct thr    *next;                // next in queue
    long           id;
    /* scheduler parameters */
    long           nice;
    long           class;
    long           prio;
    long           interact;
    long           runtime;
    /* system call context */
    struct syscall syscall;
} PACK();

/* process */

struct proc {
    struct thr       *thr;              // current running thread
    /* round-robin queue */
    struct thrq       thrq;             // queue of ready threads
    /* page directory */
    pde_t            *pdir;
    /* kernel stack */
    uint8_t          *kstk;             // kernel-mode stack (wired)
    long              class;
    /* memory attributes */
    uint8_t          *brk;
    /* process credentials */
    pid_t             pid;              // process ID
    pid_t             parent;           // parent process
    long              nthr;             // # of threads
    uid_t             ruid;             // real user ID
    gid_t             rgid;             // real group ID
    uid_t             euid;             // effective user ID
    gid_t             egid;             // effective group ID
    /* descriptors */
    desc_t           *dtab;
    desc_t           *dtab2;
    /* signal state */
    sigset_t          sigmask;
    sigset_t          sigpend;
    signalhandler_t  *sigvec[NSIG];
    /* runtime arguments */
    int               argc;
    char            **argv;
    char            **envp;
    /* memory management */
    struct slabhdr   *vmtab[PTRBITS];
    /* event queue */
//    struct ringbuf   evbuf;
} PACK() ALIGNED(PAGESIZE);

/* memory region */
struct memreg {
    struct perm   perm;
    unsigned long flg;
    uintptr_t     base;
    unsigned long size;
};

/* I/O node */
struct node {
    struct perm   perm;
    unsigned long type;
    desc_t        desc;
    unsigned long flg;  // NODESEEKBIT etc.
    /* TODO */
};

#endif /* __KERN_OBJ_H__ */

