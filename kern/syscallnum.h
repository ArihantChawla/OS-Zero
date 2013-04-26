/* notice; this header file must be assembler-safe :) */

#ifndef __KERN_SYSCALLNUM_H__
#define __KERN_SYSCALLNUM_H__

#define SYS_NONE       0
#define SYS_HALT       1
#define SYS_SYSCTL     2
#define SYS_EXIT       3
#define SYS_ABORT      4
#define SYS_FORK       5
#define SYS_EXEC       6
#define SYS_THROP      7
#define SYS_PCTL       8
#define SYS_SIGOP      9
#define SYS_BRK       10
#define SYS_MAP       11
#define SYS_UMAP      12
#define SYS_BMAP      13
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
#define SYS_READAHEAD 31
#define SYS_IOCTL     32
#define SYS_FCTL      33
#define SYS_POLL      34
#define SYS_SELECT    35
#define NSYSCALL      36

#endif /* __KERN_SYSCALLNUM_H__ */

