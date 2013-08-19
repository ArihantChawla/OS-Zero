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
#define SYS_SHMGET    13
#define SYS_SHMAT     14
#define SYS_SHMDT     15
#define SYS_SHMCTL    16
#define SYS_MNT       17
#define SYS_UMNT      18
#define SYS_READDIR   19
#define SYS_OPEN      20
#define SYS_TRUNC     21
#define SYS_CLOSE     22
#define SYS_READ      23
#define SYS_READV     24
#define SYS_WRITE     25
#define SYS_WRITEV    26
#define SYS_SEEK      27
#define SYS_FALLOC    28
#define SYS_STAT      29
#define SYS_READAHEAD 30
#define SYS_IOCTL     31
#define SYS_FCTL      32
#define SYS_POLL      33
#define SYS_SELECT    34
#define NSYSCALL      35

#endif /* __KERN_SYSCALLNUM_H__ */

