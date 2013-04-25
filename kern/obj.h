#ifndef __KERN_OBJ_H__
#define __KERN_OBJ_H__

#include <stdint.h>
#include <signal.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/types.h>
#include <kern/types.h>
#include <kern/syscall.h>

/* user + group credentials */
struct cred {
    long uid;
    long gid;
};

struct perm {
    struct cred owner;  // user + group
    long        flg;    // permission bits
};

struct thr {
    struct m_tcb  m_tcb;
    long          id;
    long          nice;
    struct proc  *proc;
    struct thr   *prev;
    struct thr   *next;
    long          class;
    long          prio;
    long          interact;
    long          runtime;
} PACK();

struct proc {
    struct thr       *thr;               // current thread
    /* wait channel */
    void             *wchan;             // wait channel
    /* page directory */
    pde_t            *pdir;
    /* kernel stack */
    uint8_t          *kstk;               // kernel-mode stack (wired)
    long              state;
    long              class;
    /* memory attributes */
    uint8_t          *brk;
    /* process credentials */
    pid_t             pid;                // process ID
    pid_t             parent;             // parent process
    long              nthr;               // # of threads
    uid_t             ruid;               // real user ID
    gid_t             rgid;               // real group ID
    uid_t             euid;               // effective user ID
    gid_t             egid;               // effective group ID
    /* descriptors */
    uintptr_t        *dtab;
    uintptr_t        *dtab2;
    /* signal state */
    sigset_t          sigmask;
    sigset_t          sigpend;
    signalhandler_t  *sigvec[NSIG];
    /* shell arguments */
    int               argc;
    char            **argv;
    char            **envp;
    /* system call context */
    struct syscall    syscall;
    struct slabhdr   *vmtab[PTRBITS];
    /* event queue */
//    struct ringbuf   evbuf;
} PACK() ALIGNED(PAGESIZE);

struct memreg {
    struct perm   perm;
    unsigned long flg;
    uintptr_t     base;
    unsigned long size;
};

struct node {
    unsigned long type;
    desc_t        desc;
    unsigned long flg;  // NODESEEKBIT etc.
    /* TODO */
};

#endif /* __KERN_OBJ_H__ */

