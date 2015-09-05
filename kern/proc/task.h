#ifndef __KERN_PROC_TASK_H__
#define __KERN_PROC_TASK_H__

long taskgetpid(void);
void taskfreepid(long id);

#define KERNPID 0
#define INITPID 1
#define MEMPID  2
#define GPUPID  3

#define THRSTKSIZE (512 * 1024)

struct pid {
    volatile long  lk;
    long           id;
    struct pid    *prev;
    struct pid    *next;
};

struct pidq {
    struct pid *head;
    struct pid *tail;
};

#endif /* __KERN_PROC_TASK_H__ */

