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
#define SYS_MHINT     12
#define SYS_MCTL      13
#define SYS_SHMGET    14
#define SYS_SHMAT     15
#define SYS_SHMDT     16
#define SYS_SHMCTL    17
#define SYS_MNT       18
#define SYS_UMNT      19
#define SYS_READDIR   20
#define SYS_OPEN      21
#define SYS_TRUNC     22
#define SYS_CLOSE     23
#define SYS_READ      24
#define SYS_READV     25
#define SYS_WRITE     26
#define SYS_WRITEV    27
#define SYS_SEEK      28
#define SYS_FALLOC    29
#define SYS_STAT      30
#define SYS_IOCTL     31
#define SYS_FCTL      32
#define SYS_POLL      33
#define SYS_SELECT    34
#define SYS_SEMINIT   35
#define SYS_SEMUP     36
#define SYS_SEMDOWN   37
#define SYS_SEMOP     38
#define SYS_MQINIT    39
#define SYS_MQSEND    40
#define SYS_MQRECV    41
#define SYS_MQCTL     42
#define SYS_EVREG     43
#define SYS_EVSEND    44
#define SYS_EVRECV    45
#define SYS_EVCTL     46

#define NSYSCALL      45

#endif /* __KERN_SYSCALLNUM_H__ */

