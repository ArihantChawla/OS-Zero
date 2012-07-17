#ifndef __KERN_SYSCALL_H__
#define __KERN_SYSCALL_H__

#include <stdint.h>
#include <zero/param.h>

/*
 * system call interface
 * ---------------------
 * - readdir() could probably be implemented in terms of ioctl().
 * - networking/socket support shall be added after the system is implemented
 *   for simple desktop use
 *  - wire() may not be feasible; mhint() might be
 *
 * process interface
 * -----------------
 * void halt(long flg);
 * long exit(long val, int flg);
 * void abort(void);
 * long fork(long flg);
 * long exec(char *file, char *argv[], char *envp[]);
 * long throp(long cmd, long parm, void *arg);
 * long pctl(long cmd, long parm, void *arg);
 * long sigop(long pid, long cmd, void *arg);
 *
 * memory interface
 * ----------------
 * long  brk(void *adr);
 * void *map(long desc, long flg, struct map *parm);
 * long  umap(void *addr, size_t sz);
 * void *bmap(long desc, size_t sz);
 *
 * shared memory
 * -------------
 * long  shmget(long id, size_t sz, long flg);
 * void *shmat(long id, void *adr, long flg);
 * long  shmdt(void *adr);
 * long  shmctl(long id, long cmd, void *arg);
 *
 * I/O interface
 * -------------
 * long mnt(char *path1, char *path2, void *arg);
 * long umnt(char *path, long flg);
 * long readdir(long desc, struct dirent *dirent, long count);
 * long open(char *path, long flg, long mode);
 * long trunc(char *path, off_t len);
 * long close(long desc);
 * long read(long desc, void *buf, size_t nb);
 * long readv(long desc, long nargs, void *args);
 * long write(long desc, void *buf, size_t nb);
 * long writev(long desc, long nargs, void *args);
 * long seek(long desc, off_t *ofs, long whence);
 * long falloc(long desc, off_t *ofs, off_t *len);
 * long stat(char *path, struct stat *buf, long flg);
 * long ioctl(long desc, long cmd, void *arg);
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
#define EXIT_SYNC       0x01U   // synchronize standard descriptors
#define EXIT_DUMP       0x02U   // dump core at termination
#define EXIT_TRACE      0x04U   // trace execution at termination
#define EXIT_PROFRES    0x08U   // collect resource usage statistics

/* fork() flags */
#define FORK_COPYONWR   0x01U   // copy on write
#define FORK_SHARMEM    0x02U   // vfork()

/* thread interface */

/* throp() commands */
#define THR_NEW         0x01    // pthread_create()
#define THR_JOIN        0x02    // pthread_join()
#define THR_DETACH      0x03    // pthread_detach()
#define THR_EXIT        0x04    // pthread_exit()
#define THR_MTXOP       0x05    // create, destroy, trylock, lock, unlock
#define THR_CLEANOP     0x06    // cleanup; push/pop handlers etc.
#define THR_KEYOP       0x07    // create, delete
#define THR_CONDOP      0x08    // condition operations (signal, broadcast, ...)
#define THR_SYSOP       0x09    // atfork, sigmask, sched, scope
#define THR_STKOP       0x0a    // stack; addr, size, guardsize
#define THR_RTOP        0x0b    // realtime thread settings
#define THR_SETATTR     0x0c    // set other attributes

/* pctl() commands */
#define PROC_WAIT       0x01    // wait()
#define PROC_USLEEP     0x02
#define PROC_NANOSLEEP  0x03
#define PROC_ATTACH     0x04    // attach debugger
#define PROC_DETACH     0x05    // detach debugger
#define PROC_DUMP       0x06    // dump core
#define PROC_TRACE      0x07    // trace execution
#define PROC_PROF       0x08    // profile execution
/* pctl() parameters */
/* PROC_WAIT flags */
#define PROC_WAITPID    0x01    // wait for pid
#define PROC_WAITCLD    0x02    // wait for children in the group pid
#define PROC_WAITGRP    0x04    // wait for children in the group of caller
#define PROC_WAITANY    0x08    // wait for any child process

struct _procwait {
    long  pid;  // who to wait for
    long *stat; // storage for exit status
    void *dat;  // rusage etc.
};

/* signal interface */

/* sigop() commands */
#define SIG_WAIT        0x01    // pause()
#define SIG_SETFUNC     0x02    // signal()/sigaction()
#define SIG_SEND        0x03    // raise() etc.
#define SIG_SETSTK      0x04    // sigaltstack()
#define SIG_SUSPEND     0x05    // sigsuspend()
/* sigop() arguments */
/* ----------------- */
/* SIG_SETFUNC */
#define SIG_FUNCDEFOP   (void *)0x01    // SIG_DFL
#define SIG_FUNCIGNORE  (void *)0x02    // SIG_IGN
/* SIG_SEND */
#define SIG_SELF       -0x01    // raise()
#define SIG_CLD        -0x02    // send to children
#define SIG_GRP        -0x03    // send to group
#define SIG_PROPCLD     0x01    // propagate to child processes
#define SIG_PROPGRP     0x02    // propagate to process group
/* SIG_PAUSE */
#define SIG_EXIT        0x01    // exit process on signal
#define SIG_DUMP        0x02    // dump core on signal

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

#endif /* __KERN_SYSCALL_H__ */

