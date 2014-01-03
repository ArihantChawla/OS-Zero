/* NOTE: this header file must be assembler-safe :) */

#ifndef __KERN_SYSCALLNUM_H__
#define __KERN_SYSCALLNUM_H__

#define SYS_NONE       0
#define SYS_SYSCTL     1
#define SYS_EXIT       2
#define SYS_ABORT      3
#define SYS_FORK       4
#define SYS_EXEC       5
#define SYS_THROP      6
#define SYS_PCTL       7
#define SYS_SIGOP      8
#define SYS_BRK        9
#define SYS_MAP       10
#define SYS_UMAP      11
#define SYS_SHMGET    12
#define SYS_SHMAT     13
#define SYS_SHMDT     14
#define SYS_SHMCTL    15
#define SYS_MNT       16
#define SYS_UMNT      17
#define SYS_READDIR   18
#define SYS_OPEN      19
#define SYS_TRUNC     20
#define SYS_CLOSE     21
#define SYS_READ      22
#define SYS_READV     23
#define SYS_WRITE     24
#define SYS_WRITEV    25
#define SYS_SEEK      26
#define SYS_FALLOC    27
#define SYS_STAT      28
#define SYS_IOCTL     29
#define SYS_FCTL      30
#define SYS_POLL      31
#define SYS_SELECT    32
#define SYS_SEMINIT   33
#define SYS_SEMUP     34
#define SYS_SEMDOWN   35
#define SYS_SEMOP     36
#define SYS_MQINIT    37
#define SYS_MQSEND    38
#define SYS_MQRECV    39
#define SYS_MQOP      40
#define SYS_EVREG     41
#define SYS_EVSEND    42
#define SYS_EVRECV    43
#define SYS_EVOP      44

#define NSYSCALL      45

#endif /* __KERN_SYSCALLNUM_H__ */

