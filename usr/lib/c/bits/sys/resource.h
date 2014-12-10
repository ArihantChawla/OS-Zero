#ifndef __BITS_SYS_RESOURCE_H__
#define __BITS_SYS_RESOURCE_H__

#define RUSAGE_SELF     0
#define RUSAGE_CHILDREN 1
#if (USEGNU)
#define RUSAGE_THREAD   2
#endif

/* which argument for set/getpriority() */
#define PRIO_PROCESS    1
#define PRIO_PGRP       2
#define PRIO_USER       3

/* resource limit special values */
#define RLIM_INFINITY   0ULL /* no limit */
#define RLIM_SAVED_MAX  0ULL
#define RLIM_SAVED_CUR  0ULL

/* which argument for set/getrlimit() */
#define RLIMIT_CORE     0 // size of core file in bytes
#define RLIMIT_CPU      1 // CPU time per process in seconds (SIGXCPU)
#define RLIMIT_DATA     2 // data segment size in bytes
#define RLIMIT_FSIZE    3 // file size in bytes
#define RLIMIT_NOFILE   4 // # of open files
#define RLIMIT_STACK    5 // stack size in bytes
#define RLIMIT_AS       6 // address space size

#endif /* __BITS_SYS_RESOURCE_H__ */

