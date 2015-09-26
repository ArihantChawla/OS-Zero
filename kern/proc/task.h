#ifndef __KERN_PROC_TASK_H__
#define __KERN_PROC_TASK_H__

#include <kern/conf.h>
#include <stddef.h>
#include <sys/types.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <zero/list.h>
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

//extern struct m_cpuinfo cpuinfo;

#if (ZEROSCHED)
FASTCALL struct task * taskpick(struct task *task);
#endif
FASTCALL void          tasksave(struct task *task);
FASTCALL void          taskjmp(struct task *task);

#define __KERNEL__ 1
//#include <zero/mtx.h>

/* process states */
#define TASKNEW     0
#define TASKREADY   1
#define TASKWAIT    2
#define TASKSLEEP   3
#define TASKSTOPPED 4
#define TASKZOMBIE  5
#define TASKNSTATE  6

struct taskstk {
    uint8_t *top;
    void    *sp;
    void    *base;
    size_t   size;
};

/* process or thread attributes */
struct task {
    /* thread control block */
    struct m_tcb     m_tcb;             // context
    /* scheduler parameters */
    long             id;                // task ID
    long             prio;              // priority; < 0 for SCHEDFIFO realtime
    long             nice;              // priority adjustment
    long             sched;             // thread scheduler class
    /* linkage */
    struct proc     *proc;              // parent/owner process
    struct task     *prev;              // previous in queue
    struct task     *next;              // next in queue
    /* state */
    long             state;             // thread state
    /* wait channel */
    uintptr_t        wchan;             // wait channel
    time_t           waketm;            // wakeup time for sleeping tasks
    /* stacks */
    struct taskstk   ustk;
    struct taskstk   kstk;
    struct taskstk   altstk;
#if 0
    uint8_t         *ustk;              // user-mode stack
    uint8_t         *kstk;              // kernel-mode stack
#endif
//    long           interact;
    long             runtime;           // run time
    /* system call context */
    struct sysctx    sysctx;            // current system call
    int              errno;             // system call error code
};

#if (PTRSIZE == 8)
#define NLVLTASKLOG2 16
#elif (PTRSIZE == 4)
#define NLVLTASKLOG2 8
#endif
#define NLVL0TASK    (1 << NLVLTASKLOG2)
#define NLVL1TASK    (1 << NLVLTASKLOG2)
#define NLVL2TASK    (1 << NLVLTASKLOG2)
#define NLVL3TASK    (1 << NLVLTASKLOG2)
#define TASKNKEY     4

#define taskwaitkey0(wc)                                                \
    (((wc) >> (3 * NLVLTASKLOG2)) & ((1UL << NLVLTASKLOG2) - 1))
#define taskwaitkey1(wc)                                                \
    (((wc) >> (2 * NLVLTASKLOG2)) & ((1UL << NLVLTASKLOG2) - 1))
#define taskwaitkey2(wc)                                                \
    (((wc) >> (1 * NLVLTASKLOG2)) & ((1UL << NLVLTASKLOG2) - 1))
#define taskwaitkey3(wc)                                                \
    ((wc) & ((1UL << NLVLTASKLOG2) - 1))

struct tasktab {
    long         nref;
    struct task *tab;
};

struct taskqueue {
    struct task *prev;
    struct task *next;
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

