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
#include <kern/cpu.h>
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

#define TASKNLVL0DL      (1U << 16)
#define TASKNLVL1DL      (1U << 8)
#define TASKNLVL2DL      (1U << 8)
#define TASKNDLKEY       3

#define TASKDEADLINEMAPNWORD (TASKNLVL0DL / (CHAR_BIT * sizeof(long)))
#define TASKREADYMAPNWORD    max(SCHEDNQUEUE / (CHAR_BIT * sizeof(long)), \
                                 CLSIZE / sizeof(long))
#define TASKIDLEMAPNWORD     max(SCHEDNIDLE / (CHAR_BIT * sizeof(long)), \
                                 CLSIZE / sizeof(long))
#define TASKLOADMAPNWORD     max((SCHEDNTOTALQUEUE) / CHAR_BIT * sizeof(long), \
                                 CLSIZE / sizeof(long))
#define TASKIDLECPUMAPNWORD  max(NCPU / (CHAR_BIT * sizeof(long)),      \
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
#define TASKHASINPUT (1 << 0)   // pending HID input
#define TASKISBOUND  (1 << 1)   // bound to a processor, cannot migrate
#define TASKXFERABLE (1 << 2)   // task was added as transferable
#define TASKCATCHSIG (1 << 3)   // sleeping thread awakened by signals
struct task {
    /* thread control block - KEEP THIS FIRST in the structure */
    struct m_task   m_task;             // machine thread control block
    /* scheduler parameters */
    volatile long   lk;
    long            sched;              // thread scheduler class
    long            schedflg;           // received user input [interrupt]
    long            runprio;            // current priority
    long            prio;               // base priority
    long            sysprio;            // kernel-mode priority
//    long            nice;               // priority adjustment
    long            state;              // thread state
    long            score;              // interactivity score
    long            cpu;                // CPU-affinity
    long            slice;              // timeslice in ticks
    long            runtime;            // # of ticks run
    long            slptime;            // # of ticks  of slept voluntarily
    long            slptick;            // ID of tick when sleeping started
    long            ntick;              // # of scheduler ticks received
    long            lastrun;            // last tick we ran on
    long            firstrun;           // first tick we ran on
    long            ntickleft;          // # of remaining ticks of slice
    long            lasttick;           // real last tick for affinity
    uintptr_t       waitchan;           // wait channel
    time_t          timelim;            // wakeup time or deadline
    /* linkage */
    struct proc    *proc;               // parent/owner process
    struct task    *prev;               // previous in queue
    struct task    *next;               // next in queue
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

#if (PTRSIZE == 8)
#define TASKNLVLWAITLOG2 16
#elif (PTRSIZE == 4)
#define TASKNLVLWAITLOG2 8
#endif
#define TASKNLVL0WAIT    (1 << TASKNLVLWAITLOG2)
#define TASKNLVL1WAIT    (1 << TASKNLVLWAITLOG2)
#define TASKNLVL2WAIT    (1 << TASKNLVLWAITLOG2)
#define TASKNLVL3WAIT    (1 << TASKNLVLWAITLOG2)
#define TASKNWAITKEY     4

#define taskwaitkey0(wc)                                                \
    (((wc) >> (3 * TASKNLVLWAITLOG2)) & ((1UL << TASKNLVLWAITLOG2) - 1))
#define taskwaitkey1(wc)                                                \
    (((wc) >> (2 * TASKNLVLWAITLOG2)) & ((1UL << TASKNLVLWAITLOG2) - 1))
#define taskwaitkey2(wc)                                                \
    (((wc) >> (1 * TASKNLVLWAITLOG2)) & ((1UL << TASKNLVLWAITLOG2) - 1))
#define taskwaitkey3(wc)                                                \
    ((wc) & ((1UL << TASKNLVLWAITLOG2) - 1))

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
    struct task   *list;
    long           load;
    uint8_t        pad[CLSIZE - 2 * sizeof(long) - sizeof(struct task *)];
};

struct taskqueueset {
    volatile long   lk;
    long           *curmap;
    long           *nextmap;
    long           *idlemap;
    long           *loadmap;
    struct task   **cur;
    struct task   **next;
    struct task   **idle;
    uint8_t         pad[CLSIZE - sizeof(long) - 6 * sizeof(void *)];
};

#if 0
struct taskqueuehdr {
    volatile long     lk;
    struct taskqueue *tab;
    uint8_t           pad[CLSIZE - sizeof(long) - sizeof(struct taskqueue *)];
};
#endif

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

