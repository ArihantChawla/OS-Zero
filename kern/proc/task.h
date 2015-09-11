#ifndef __KERN_PROC_TASK_H__
#define __KERN_PROC_TASK_H__

#include <kern/conf.h>
#include <stddef.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/types.h>
#include <kern/list.h>
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
extern FASTCALL struct task * taskpick(void);
#endif
ASMLINK void                  tasksave(struct task *task, long retadr, long fp);
FASTCALL void                 taskjmp(struct task *task);

#define __KERNEL__ 1
//#include <zero/mtx.h>

/* process states */
#define TASKCREATE  0
#define TASKUSER    1
#define TASKKERNEL  2
#define TASKREADY   3
#define TASKWAIT    4
#define TASKSTOP    5
#define TASKZOMBIE  6
#define TASKNSTATE  7

/* process or thread attributes */
struct task {
    /* thread control block */
    struct m_tcb   m_tcb;               // context
    /* scheduler parameters */
    long           prio;                // priority; < 0 for SCHEDFIFO
    long           nice;                // priority adjustment
    long           sched;               // thread class
    /* linkage */
    struct proc   *parent;              // parent/owner process
    struct task   *prev;                // previous in queue
    struct task   *next;                // next in queue
    /* state */
    long           state;               // thread state
    /* wait channel */
    uintptr_t      wchan;               // wait channel
    long           id;                  // process or thread id
    /* stacks */
    uint8_t       *ustk;                // user-mode stack
    uint8_t       *kstk;                // kernel-mode stack
//    long           interact;
    long           runtime;             // run time
    /* system call context */
    struct sysop   sysop;               // current system call
    int            errno;               // system call error code
};

struct taskqueue {
    volatile long  lk;
    struct task   *head;
    struct task   *tail;
};

#if (LONGSIZE == 8)
#define NLVLTASKLOG2 16
#elif (LONGSIZE == 4)
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

struct taskwait {
    volatile long  lk;
    long           nref;
    void          *ptr;
};

#if 0
#if (!QEMU)
#define FPUCTX 1
#endif
#endif

long taskgetid(void);
void taskfreeid(long id);

#define KERNPID 0
#define INITPID 1
#define MEMPID  2
#define GPUPID  3

#define THRSTKSIZE  (512 * 1024)

struct taskid {
    volatile long  lk;
    long           id;
    struct taskid *prev;
    struct taskid *next;
};

struct taskidq {
    struct taskid *head;
    struct taskid *tail;
};

#define PIDSPEC_PID  0
#define PIDSPEC_TGID 1
#define PIDSPEC_PGID 2
#define PIDSPEC_SID  3
#define PIDSPEC_MAX  4

struct pid {
    long             num;
    volatile long    cnt;
    struct task     *task;
    struct listhead  list;
    struct listhead  hash;
};

#endif /* __KERN_PROC_TASK_H__ */

