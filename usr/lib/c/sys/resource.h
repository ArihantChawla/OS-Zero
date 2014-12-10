#ifndef __SYS_RESOURCE_H__
#define __SYS_RESOURCE_H__

#include <features.h>
#include <sys/types.h>

/* resource usage statistics */
extern int getrusage(int who, struct rusage *rusage);
/* nice value routines */
extern int getpriority(int which, id_t who);
extern int setpriority(int which, id_t who, int prio);
/* resource limit routines */
extern int getrlimit(int which, struct rlimit *lim);
extern int setrlimit(int which, const struct rlimit *lim);

typedef unsigned long long rlim_t;

struct rlimit {
    rlim_t rlim_cur; // soft limit
    rlim_t rlim_max; // hard limit
};

struct rusage {
    struct timeval ru_utime;    /* user CPU time used */
    struct timeval ru_stime;    /* system CPU time used */
    long           ru_maxrss;   /* maximum resident set size */
    long           ru_ixrss;    /* integral shared memory size */
    long           ru_idrss;    /* integral unshared data size */
    long           ru_isrss;    /* integral unshared stack size */
    long           ru_minflt;   /* page reclaims (soft page faults) */
    long           ru_majflt;   /* page faults (hard page faults) */
    long           ru_nswap;    /* swaps */
    long           ru_inblock;  /* block input operations */
    long           ru_oublock;  /* block output operations */
    long           ru_msgsnd;   /* IPC messages sent */
    long           ru_msgrcv;   /* IPC messages received */
    long           ru_nsignals; /* signals received */
    long           ru_nvcsw;    /* voluntary context switches */
    long           ru_nivcsw;   /* involuntary context switches */
};

#endif /* __SYS_RESOURCE_H__ */

