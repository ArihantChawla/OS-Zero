#ifndef __KERN_PROC_TASK_H__
#define __KERN_PROC_TASK_H__

#include <kern/conf.h>
#include <time.h>
#include <mach/param.h>
#include <mach/types.h>
#include <kern/syscall.h>
#include <mt/tktlk.h>
#define TASK_LK_T    zerotktlk
#define tasklk(lp)   tktlk(lp)
#define taskunlk(lp) tktunlk(lp)

typedef int16_t      taskid_t;

/* process states */
#define TASKNEW      0
#define TASKREADY    1
#define TASKSLEEPING 2
#define TASKWAITING  3 // NEW
#define TASKSTOPPED  4
#define TASKZOMBIE   5
#define TASKNSTATE   6

struct taskstk {
    uint8_t *top;
    void    *sp;
    void    *base;
    size_t   size;
};

#define TASKWAITHASHTABITEMS 29
#define TASKWAITHASHTABSIZE  (32 * PTRSIZE)
struct taskwaithashtab {
    uintptr_t               chan;
    struct taskwaithashtab *next;
    long                    n;
    struct task            *buf[TASKWAITHASHTABITEMS];
};

#define TASKSWAITHASHITEMS (1U << TASKWAITHASHBITS)
#define TASKWAITHASHBITS   10

#define TASKALLOCSIZE 1024
#define TASKBUFITEMS  32
/* process or thread attributes */
/* bits for schedflg-member */
#define TASKHASINPUT (1 << 0)   // pending HID input
#define TASKISBOUND  (1 << 1)   // bound to a processor, cannot migrate
#define TASKXFERABLE (1 << 2)   // task was added as transferable
#define TASKCATCHSIG (1 << 3)   // sleeping thread awakened by signals
struct task {
    /* thread control block - KEEP THIS FIRST in the structure */
    struct m_task    m_task;            // machine thread control block
    long             id;                // task ID
    /* linkage */
    struct proc     *proc;              // parent/owner process
    struct task     *prev;              // previous in queue
    struct task     *next;              // next in queue
    /* execution state */
    long             argc;              // # of command-line arguments
    char           **argv;              // [textual] command-line arguments
    long             nenv;              // # of environment strings
    char           **envp;              // environment strings
    /* system call context */
    int              errnum;            // errno
    struct sysctx    sysctx;            // current system call
    /* signal state */
    sigset_t         sigmask;           // signal mask
    sigset_t         sigpend;           // pending signals
    struct siginfo **sigqueue;          // info structures for pending signals
    /* scheduler parameters */
    TASK_LK_T        lk;
    long             unit;              // CPU-affinity
    long             sched;             // thread scheduler class
    long             flg;               // received user input [interrupt]
    long             runprio;           // current priority
    long             prio;              // base priority
    long             sysprio;           // kernel-mode priority
    long             nice;              // priority adjustment
    long             state;             // thread state
    long             score;             // interactivity score
    long             slice;             // timeslice in ticks
    long             runtime;           // # of ticks run
    long             slptime;           // # of ticks slept voluntarily
    long             slptick;           // ID of tick when sleeping started
    long             ntick;             // # of scheduler ticks received
    long             lastrun;           // last tick we ran on
    long             firstrun;          // first tick we ran on
    long             ntickleft;         // # of remaining ticks of slice
    long             lasttick;          // real last tick for affinity
    uintptr_t        waitchan;          // wait channel
    time_t           timelim;           // wakeup time or deadline
};

extern struct task k_tasktab[TASKSMAX];

#if 0
#if (PTRSIZE == 8)
#define TASKNLVLWAITLOG2 16
#elif (PTRSIZE == 4)
#define TASKNLVLWAITLOG2 8
#endif
#define TASKNLVL0WAIT    (1 << TASKNLVLWAITLOG2)
#define TASKNLVL1WAIT    (1 << TASKNLVLWAITLOG2)
#define TASKNLVL2WAIT    (1 << TASKNLVLWAITLOG2)
#define TASKNLVL3WAIT    (1 << TASKNLVLWAITLOG2)
//#define TASKNWAITKEY     4

#define taskwaitkey0(wc)                                                \
    (((wc) >> (3 * TASKNLVLWAITLOG2)) & ((1UL << TASKNLVLWAITLOG2) - 1))
#define taskwaitkey1(wc)                                                \
    (((wc) >> (2 * TASKNLVLWAITLOG2)) & ((1UL << TASKNLVLWAITLOG2) - 1))
#define taskwaitkey2(wc)                                                \
    (((wc) >> (1 * TASKNLVLWAITLOG2)) & ((1UL << TASKNLVLWAITLOG2) - 1))
#define taskwaitkey3(wc)                                                \
    ((wc) & ((1UL << TASKNLVLWAITLOG2) - 1))

struct tasktabl0 {
    m_atomic_t      lk;
    long            nref;
    struct tasktab *tab;
    uint8_t         _pad[CLSIZE - 2 * sizeof(long) - sizeof(struct tasktab *)];
};

struct tasktab {
    long         nref;
    struct task *tab;
};

/* this should be a single (aligned) cacheline */
struct taskqueue {
    m_atomic_t   lk;
    struct task *list;
    uint8_t      _pad[CLSIZE - sizeof(long) - sizeof(struct task *)];
};
#endif

taskid_t taskgetid(void);
void     taskputid(taskid_t id);

#define THRSTKSIZE  (64 * 1024)

#if 0
struct taskid {
    m_atomic_t     lk;
    long           id;
    struct taskid *prev;
    struct taskid *next;
};
#endif

#define PIDSPEC_PID  0
#define PIDSPEC_TGID 1
#define PIDSPEC_PGID 2
#define PIDSPEC_SID  3
#define PIDSPEC_MAX  4

struct pid {
    long         num;
    m_atomic_t   cnt;
    struct task *task;
    struct pid  *list;
    struct pid  *hash;
};

extern void tasksetsleep(struct task *task);
extern void tasksetwait(struct task *task);

#endif /* __KERN_PROC_TASK_H__ */

