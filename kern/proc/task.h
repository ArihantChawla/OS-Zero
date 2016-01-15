#ifndef __KERN_PROC_TASK_H__
#define __KERN_PROC_TASK_H__

#include <kern/conf.h>
#include <stddef.h>
#include <sys/types.h>
#include <zero/cdefs.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/trix.h>
//#include <zero/list.h>
#include <kern/syscall.h>
#include <kern/unit/x86/cpu.h>
#if (defined(__i386__) || defined(__i486__)                             \
     || defined(__i586__) || defined(__i686__)                          \
     && (!defined(__x86_64__) && !defined(__amd64__)))
#include <kern/unit/ia32/asm.h>
#elif defined(__i386__)
#include <kern/unit/ia32/task.h>
#elif defined(__arm__)
#include <kern/unit/arm/asm.h>
#elif defined(__ppc__)
#include <kern/unit/ppc/asm.h>
#endif

#define TASKDEADLINEBITMAPNWORD ((1UL << 16) / sizeof(long))
#define TASKREADYBITMAPNWORD    max(SCHEDNTOTALQUEUE / sizeof(long),    \
                                    CLSIZE / sizeof(long))

#define __errnoloc() (&k_curtask->errnum)

//extern struct m_cpuinfo cpuinfo;

#define __KERNEL__ 1
//#include <zero/mtx.h>

/* process states */
#define TASKNEW      0
#define TASKREADY    1
#define TASKSLEEPING 2
#define TASKSTOPPED  3
#define TASKZOMBIE   4
#define TASKNSTATE   5

struct taskstk {
    uint8_t *top;
    void    *sp;
    void    *base;
    size_t   size;
};

/* process or thread attributes */
/* bits for schedflg-member */
#define TASKHASINPUT (1 << 0)
struct task {
    /* thread control block - KEEP THIS FIRST in the structure */
    struct m_task   m_task;             // machine-thread control block
    /* scheduler parameters */
    long            sched;              // thread scheduler class
    long            schedflg;           // received user input [interrupt]
    long            prio;               // priority; < 0 for SCHEDFIFO realtime
    long            nice;               // priority adjustment
    long            state;              // thread state
    unsigned long   runtime;            // # of milliseconds run
    unsigned long   slptime;            // amount of voluntary sleep
    unsigned long   ntick;              // # of scheduler ticks received
    unsigned long   firsttick;
    unsigned long   lasttick;
    time_t          timelim;            // wakeup time or deadline
    /* linkage */
    struct proc    *proc;               // parent/owner process
    struct task    *prev;               // previous in queue
    struct task    *next;               // next in queue
    uintptr_t       wtchan;             // wait channel
    long            id;                 // task ID
    /* system call context */
    struct sysctx   sysctx;             // current system call
    /* signal state */
    sigset_t        sigmask;            // signal mask
    sigset_t        sigpend;            // pending signals
    struct siginfo *sigqueue[NSIG];     // info structures for pending signals
#if 0
    /* stack information */
    struct taskstk  ustk;               // user-mode stack
    struct taskstk  kstk;               // system-mode stack
    struct taskstk  altstk;             // alternative [signal] stack
#endif
    int             errnum;             // errno
//    long           interact;
};

#define NLVL0DL      (1U << 16)
#define NLVL1DL      (1U << 8)
#define NLVL2DL      (1U << 8)
#define DLNKEY       3

#if (PTRSIZE == 8)
#define NLVLWAITLOG2 16
#elif (PTRSIZE == 4)
#define NLVLWAITLOG2 8
#endif
#define NLVL0WAIT    (1 << NLVLWAITLOG2)
#define NLVL1WAIT    (1 << NLVLWAITLOG2)
#define NLVL2WAIT    (1 << NLVLWAITLOG2)
#define NLVL3WAIT    (1 << NLVLWAITLOG2)
#define WAITNKEY     4

#define taskwaitkey0(wc)                                                \
    (((wc) >> (3 * NLVLWAITLOG2)) & ((1UL << NLVLWAITLOG2) - 1))
#define taskwaitkey1(wc)                                                \
    (((wc) >> (2 * NLVLWAITLOG2)) & ((1UL << NLVLWAITLOG2) - 1))
#define taskwaitkey2(wc)                                                \
    (((wc) >> (1 * NLVLWAITLOG2)) & ((1UL << NLVLWAITLOG2) - 1))
#define taskwaitkey3(wc)                                                \
    ((wc) & ((1UL << NLVLWAITLOG2) - 1))

struct tasktabl0 {
    volatile long   lk;
    long            nref;
    struct tasktab *tab;
    uint8_t         pad[CLSIZE - 2 * sizeof(long) - sizeof(struct tasktab *)];
};

struct tasktab {
    long           nref;
    struct task   *tab;
};

/* this should be a single (aligned) cacheline */
struct taskqueue {
    volatile long  lk;
    struct task   *prev;
    struct task   *next;
    uint8_t        pad[CLSIZE - sizeof(long) - 2 * sizeof(struct task *)];
};

#if 0
#if (!QEMU)
#define FPUCTX 1
#endif
#endif

long taskgetid(void);
void taskfreeid(long id);

#define THRSTKSIZE  (64 * 1024)

struct taskid {
    volatile long  lk;
    long           id;
    struct taskid *prev;
    struct taskid *next;
    uint8_t        pad[CLSIZE - 2 * sizeof(long) - 2 * sizeof(struct taskid *)];
};

#define PIDSPEC_PID  0
#define PIDSPEC_TGID 1
#define PIDSPEC_PGID 2
#define PIDSPEC_SID  3
#define PIDSPEC_MAX  4

struct pid {
    long           num;
    volatile long  cnt;
    struct task   *task;
    struct pid    *list;
    struct pid    *hash;
};

#endif /* __KERN_PROC_TASK_H__ */

