#ifndef __KERN_SYSCALL_H__
#define __KERN_SYSCALL_H__

#include <features.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/shm.h>
#include <zero/param.h>

/*
 * system call interface
 * ---------------------
 * - networking/socket support shall be added after the system is implemented
 *   for simple desktop use
 *
 * process interface
 * -----------------
 * void sys_halt(long flg);
 * long sys_sysctl(long cmd, long parm, void *arg);
 * long sys_exit(long val, long flg);
 * void sys_abort(void);
 * long sys_fork(long flg);
 * long sys_exec(char *path, char *argv[], ...);
 * long sys_throp(long cmd, long parm, void *arg);
 * long sys_pctl(long cmd, long parm, void *arg);
 * long sys_sigop(long cmd, long parm, void *arg);
 *
 * memory interface
 * ----------------
 * long  sys_brk(void *adr);
 * void *sys_map(long desc, long flg, struct sysmem *arg);
 * - map file or anonymous memory
 * long  sys_umap(void *adr, size_t size);
 * - unmap file or anonymous memory
 * long  sys_mhint(void *adr, long flg, struct sysmem *arg);
 * - NORMAL, RANDOM, SEQUENTIAL, WILLNEED, DONTNEED, REMOVE, DONTFORK, DOFORK
 *
 * shared memory
 * -------------
 * long  sys_shmget(long key, size_t size, long flg);
 * void *sys_shmat(long id, void *adr, long flg);
 * long  sys_shmdt(void *adr);
 * long  sys_shmctl(long id, long cmd, void *arg);
 *
 * semaphores
 * ----------
 *
 * read-write locks
 * ----------------
 *
 * message queues
 * --------------
 *
 * events
 * ------
 *
 * I/O interface
 * -------------
 * long sys_mnt(char *path1, char *path2, void *arg);
 * long sys_umnt(char *path, long flg);
 * long sys_readdir(long desc, struct dirent *dirent, long count);
 * long sys_open(char *path, long flg, long mode);
 * long sys_trunc(char *path, off_t len);
 * long sys_close(long desc);
 * long sys_read(long desc, void *buf, size_t nb);
 * long sys_readv(long desc, long nargs, void *args);
 * long sys_write(long desc, void *buf, size_t nb);
 * long sys_writev(long desc, long nargs, void *args);
 * long sys_seek(long desc, off_t ofs, long whence);
 * long sys_falloc(long desc, long parm, size_t len);
 * long sys_stat(char *path, struct stat *buf, long flg);
 * long sys_fhint(long desc, long flg, struct freg *arg);
 * - NORMAL, SEQUENTIAL, RANDOM, WILLNEED, DONTNEED, NOREUSE, NONBLOCK, SYNC
 * long sys_ioctl(long desc, long cmd, void *arg);
 * long sys_fctl(long desc, long cmd, void *arg);
 * long sys_poll(struct pollfd *fds, long nfd, long timeout);
 * long sys_select(long nfds, struct select *args);
 */

/*
 * special files
 * -------------
 * /dev/null
 * /dev/zero
 * /dev/con
 * /dev/fifo
 * /dev/sock
 * /dev/mtx
 * /dev/sem
 * /dev/rwlk
 * /dev/mq
 */

/* process system calls */

/* halt() flags */
#define HALT_REBOOT     0x01U   // reboot()

/* exit() flags */
#define EXIT_DUMPACCT   0x01U

/* fork() flags */
#define FORK_VFORK      0x01U   // vfork()
#define FORK_COW        0x02U   // copy on write

/* thread interface */

/* TODO: move MTX, SEM, and COND operations under IPC */

/* throp() commands */
#define THR_NEW          1      // pthread_create()
#define THR_JOIN         2      // pthread_join()
#define THR_DETACH       3      // pthread_detach()
#define THR_EXIT         4      // pthread_exit()
#define THR_CLEANUP      5      // cleanup; pop and execute handlers etc.
#define THR_KEYOP        6      // create, delete
#define THR_SYSOP        7      // atfork, sigmask, sched, scope
#define THR_STKOP        8      // stack; addr, size, guardsize
#define THR_RTOP         9      // realtime thread settings
#define THR_SETATR      10      // set other attributes

/* pctl() commands */
#define PROC_GETPID     0x01    // getpid()
#define PROC_GETPGRP    0x02    // getpgrp()
#define PROC_WAIT       0x03    // wait()
#define PROC_USLEEP     0x04    // usleep()
#define PROC_NANOSLEEP  0x05    // nanosleep()
#define PROC_SETSCHED   0x07    // nice(), ...
#define PROC_STAT       0x08    // getrusage()
#define PROC_GETLIM     0x09    // getrlimit()
#define PROC_SETLIM     0x09    // setrlimit()
/* pctl() parameters */
#if 0
/* PROC_WAIT flags */
#define PROC_WAITPID    0x01    // wait for pid
#define PROC_WAITCLD    0x02    // wait for children in the group pid
#define PROC_WAITGRP    0x04    // wait for children in the group of caller
#define PROC_WAITANY    0x08    // wait for any child process
#endif

struct syswait {
    long  pid;  // who to wait for
    long *stat; // storage for exit status
    void *data; // rusage etc.
};

/* signal interface */

/* sigop() commands */
#define SIG_WAIT        0x01    // pause()
#define SIG_SETFUNC     0x02    // signal()/sigaction()
#define SIG_SETMASK     0x03    // sigsetmask()
#define SIG_SEND        0x04    // raise() etc.
#define SIG_SETSTK      0x05    // sigaltstack()
#define SIG_SUSPEND     0x06    // sigsuspend(), sigpause()
/* sigop() arguments */
/* ----------------- */
/* SIG_SETFUNC */
//#define SIG_DEFAULT     (void *)0x01    // SIG_DFL
//#define SIG_IGNORE      (void *)0x02    // SIG_IGN
/* SIG_SEND */
#define SIG_SELF       -0x01    // raise()
#define SIG_CLD        -0x02    // send to children
#define SIG_GRP        -0x03    // send to group
#define SIG_PROPCLD     0x01    // propagate to child processes
#define SIG_PROPGRP     0x02    // propagate to process group
/* SIG_PAUSE */
#define SIG_EXIT        0x01    // exit process on signal
#define SIG_DUMP        0x02    // dump core on signal

/* memory interface */
#define MAP_FILE        0x00000001
#define MAP_ANON        0x00000002
#define MAP_SHARED      0x00000004
#define MAP_PRIVATE     0x00000008
#define MAP_FIXED       0x00000010
#define MAP_SINGLE      0x00000020
#define MEM_NORMAL      0x00000040
#define MEM_SEQUENTIAL  0x00000080
#define MEM_RANDOM      0x00000100
#define MEM_WILLNEED    0x00000200
#define MEM_DONTNEED    0x00000400
#define MEM_DONTFORK    0x00000800

struct sysmem {
    void *base;
    long  ofs;
    long  len;
    long  perm;
};

#define IPC_CREAT       0x00000001
#define IPC_PRIVATE     0L
#define IPC_STAT        1
#define IPC_SET         2
#define IPC_RMID        3

#define SEEK_CUR        0x00
#define SEEK_BEG        0x01
#define SEEK_END        0x02

struct syscall {
    long    arg1;
    long    arg2;
    long    arg3;
#if (LONGSIZE == 4)
    int64_t arg64;
#endif    
};

struct freg {
    long desc;
    long ofs;
    long len;
    long perm;
};

/* ioctl() */
struct ioctl {
    off_t ofs;
    off_t len;
    long flg;   // NOBUF
};

struct select {
    fd_set          *readset;
    fd_set          *writeset;
    fd_set          *errorset;
    struct timespec *timeout;
    sigset_t        *sigmask;
};

#endif /* __KERN_SYSCALL_H__ */

