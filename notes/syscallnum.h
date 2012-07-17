/* notice; this header file must be assembler-safe :) */

#ifndef __KERN_SYSCALLNUM_H__
#define __KERN_SYSCALLNUM_H__

#define SYS_SHUTDOWN 1
#define SYS_REBOOT   2
#define SYS_SYSCTL   3
#define SYS_EXIT     4
#define SYS_ABORT    5
#define SYS_FORK     6
#define SYS_EXEC     7
#define SYS_THROP    8
#define SYS_PCTL     9
#define SYS_SIGOP    10
#define SYS_BRK      11
#define SYS_MAP      12
#define SYS_UMAP     13
#define SYS_BMAP     14
#define SYS_SHMGET   15
#define SYS_SHMAT    16
#define SYS_SHMDT    17
#define SYS_SHMCTL   18
#define SYS_MNT      19
#define SYS_UMNT     20
#define SYS_READDIR  21
#define SYS_OPEN     22
#define SYS_TRUNC    23
#define SYS_CLOSE    24
#define SYS_READ     25
#define SYS_READV    26
#define SYS_WRITE    27
#define SYS_WRITEV   28
#define SYS_SEEK     29
#define SYS_FALLOC   30
#define SYS_STAT     31
#define SYS_IOCTL    32
#define NSYSCALL     33

#endif /* __KERN_SYSCALLNUM_H__ */

