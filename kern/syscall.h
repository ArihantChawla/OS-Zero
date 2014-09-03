#ifndef __KERN_SYSCALL_H__
#define __KERN_SYSCALL_H__

#include <features.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <kern/perm.h>

/*
 * TODO
 * ----
 * sysctl functionality
 * --------------------
 * - commands to access system clock
 * - commands to query and set system attributes
 *   - page size, stack size, cacheline size, ...
 */

/*
 * system call interface
 * ---------------------
 * - networking/socket support shall be added after the system is implemented
 *   for simple desktop use
 *
 * process interface
 * -----------------
 * long sys_sysctl(long cmd, long parm, void *arg);
 * - perform system control actions
 * long sys_exit(long val, long flg);
 * - exit process normally
 * void sys_abort(void);
 * - exit problem abnormally
 * long sys_fork(long flg);
 * - create new process by cloning current one
 * long sys_exec(char *path, long parm, void *arg);
 * - execute program image
 * long sys_throp(long cmd, long parm, void *arg);
 * - perform thread control operations
 * long sys_pctl(long cmd, long parm, void *arg);
 * - perform process control operations
 * long sys_sigop(long cmd, long parm, void *arg);
 * - perform signal actions
 *
 * memory interface
 * ----------------
 * long  sys_brk(void *adr);
 * - set top of heap to adr
 * void *sys_map(long desc, long flg, struct sysmemreg *reg);
 * - map file or anonymous memory
 * long  sys_umap(void *adr, size_t size);
 * - unmap file or anonymous memory
 * long  sys_mhint(void *adr, long flg, struct sysmemreg *reg);
 * - hint kernel about memory region use characteristics
 * long  sys_mctl(void *adr, long flg, struct sysmemreg *reg);
 * - memory control operations; locks, permissions, etc.
 *
 * IPC
 * ---
 * key_t sys_ipcmkkey(void *arg);
 * key_t sys_ipcrelkey(key_t key);
 *
 * shared memory
 * -------------
 * uintptr_t  sys_shmget(long key, size_t size, long flg);
 * - look up shared memory segment associated with key or create a new one with key IPC_PRIVATE
 * void      *sys_shmat(uintptr_t id, void *adr, long flg);
 * - map shared memory segment to virtual address space of calling process; specifying adr is not
 *   guaranteed to succeed
 * long       sys_shmdt(void *adr);
 * - unmap shared memory segment from virtual address space of calling process
 * long       sys_shmctl(uintptr_t id, long cmd, void *arg);
 * - perform shared memory control actions
 *
 * semaphores
 * ----------
 * uintptr_t sys_mksem(long cnt);
 * - allocate semaphore and initialise it with value cnt
 * void      sys_relsem(uintptr_t id);
 * - deallocate semaphore
 * long      sys_semup(uintptr_t sem, long n);
 * - increase semaphore value by n
 * long      sys_semdown(uintptr_t sem, long n);
 * - attempt to decrease semaphore value by n
 *
 * read-write locks
 * ----------------
 * uintptr_t sys_mkrwlk(long desc, long flg, struct objreg *arg);
 * - allocate and initialise read-write lock
 * void      sys_relrwlk(long desc, long flg);
 * - deallocate read-write lock
 * uintptr_t    sys_lockrd(long desc, long flg, struct objreg *arg);
 * - acquire read-lock of object desc
 * uintptr_t sys_lockwr(long desc, long flg, struct objreg *arg);
 * - acquire write-lock on object describe
 * void      sys_unlock(uintptr_t lk);
 * - unlock a read-write lock
 *
 * message queues
 * --------------
 * uintptr_t sys_mkmq(long nprio, size_t qsize);
 * long      sys_postmsg(long prio, struct msg *arg);
 * long      sys_readmsg(long mq, struct msg *arg);
 *
 * events
 * ------
 *
 * I/O interface
 * -------------
 * long sys_mnt(char *path1, char *path2, void *arg);
 * - mount filesystem path2 to mountpoint path1
 * long sys_umnt(char *path, long flg);
 * - unmount filesystem mounted to path§
 * long sys_readdir(long desc, struct dirent *dirent, long cnt);
 * - read directory contents
 * long sys_open(char *path, long flg, long mode);
 * - open file path for operations as selected in mode§
 * long sys_trunc(char *path, off_t len);
 * - truncate file path to length len; toss contents behind len
 * long sys_close(long desc);
 * - close file refered to by desc
 * long sys_read(long desc, void *buf, size_t len);
 * - read next maximum of len data bytes from file desc into buf
 * long sys_readv(long desc, long narg, void *arg);
 * - read from several files into a buffer described by arg
 * long sys_write(long desc, void *buf, size_t len);
 * - attempt to write len data bytes from memory pointed to by buf to file desc
 * long sys_writev(long desc, long narg, void *arg);
 * - write into file desc from several buffers described by arg
 * long sys_seek(long desc, off_t ofs, long whence);
 * - set seek position of file desc
 * long sys_falloc(long desc, long parm, size_t len);
 * - attempt to preallocate len bytes of space for file desc
 * long sys_stat(char *path, struct stat *buf, long flg);
 * - query file attributes
 * long sys_fhint(long desc, long flg, struct objreg *arg);
 * - hint kernel of file usage patterns
 * long sys_ioctl(long desc, long cmd, void *arg);
 * - perform device-dependent I/O operations§
 * long sys_fctl(long desc, long cmd, void *arg);
 * - perform file control operations
 * long sys_poll(struct pollfd *fds, long nfd, long timeout);
 * long sys_select(long nfds, struct select *arg);
 */

/*
 * special files and directories
 * -----------------------------
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

/* process management system calls */

/* sys_sysctl */

/* cmd */
#define SYSCTL_HALT      0x01U  // halt() and reboot()
#define SYSCTL_SYSINFO   0x02U  // sysconf()
#define SYSCTL_SYSSTAT   0x03U  // query system statistics; getrusage()
/* parm */
/* flg value for SYSCTL_HALT */
#define SYSCTL_REBOOT    0x01U   // reboot()
/* SYSCTL_SYSINFO */
#define SYSCTL_CLSIZE    0x01U   // query system cacheline size; getclsize()
#define SYSCTL_PAGESIZE  0x02U   // getpagesize(); sysconf() with _SC_PAGESIZE/_SC_PAGE_SIZE
#define SYSCTL_DTSIZE    0x03U   // query system descriptor table size
#define SYSCTL_STKSIZE   0x03U   // query process stack size; getstksize()
#define SYSCTL_RAMSIZE   0x04U   // query system memory size; getramsize()
#define SYSCTL_NPROC     0x05U   // query or set maximum number of processes on system
#define SYSCTL_NTHR      0x06U   // query or set maximum number of threads on system
/* SYSCTL_SYSSTAT */
#define SYSCTL_UPTIME    0x01U   // query system uptime
/* SYSCTL_TIME */
#define SYSCTL_GETTIME   0x01U   // query system time
#define SYSCTL_SETTIME   0x02U   // set system time

/* exit() parm flags */
#define EXIT_DUMPACCT    0x01U   // dump system information at process exit

/* abort() parm flags */
#define ABORT_DUMPCORE   0x01U   // dump core image at abnormal process exit

/* fork() parm flags */
#define FORK_VFORK       0x01U   // vfork() semantics; share address space with parent
#define FORK_COW         0x02U   // copy on write optimisations

struct sysatexit {
	void (*func)(void);
};

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
#define PROC_ATEXIT     0x10    // atexit(), on_exit()
#if 0
/* pctl() parameters */
/* PROC_WAIT flags */
#define PROC_WAITPID    0x01    // wait for pid
#define PROC_WAITCLD    0x02    // wait for children in the group pid
#define PROC_WAITGRP    0x04    // wait for children in the group of caller
#define PROC_WAITANY    0x08    // wait for any child process
#endif

struct syswait {
    long  pid;  // processes to wait for
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

/* sys_map() */
#define MAP_FILE        0x00000001
#define MAP_ANON        0x00000002
#define MAP_SHARED      0x00000004
#define MAP_PRIVATE     0x00000008
#define MAP_FIXED       0x00000010
#define MAP_SINGLE      0x00000020
/* sys_map() and sys_mhint() */
#define MEM_NORMAL      0x00000040
#define MEM_SEQUENTIAL  0x00000080
#define MEM_RANDOM      0x00000100
#define MEM_WILLNEED    0x00000200
#define MEM_WONTNEED    0x00000400
#define MEM_DONTFORK    0x00000800
/* sys_mctl() */
/* cmd */
#define MEM_LOCK        0x01    // mlock(), mlockall()
#define MEM_UNLOCK      0x02    // munlock(), munlockall()
/* REFERENCE: <kern/perm.h> */
#define MEM_GETPERM     0x03
#define MEM_SETPERM     0x04

struct sysmemreg {
    struct perm  perm;					// permission structure
    long         cmd;					// memory command
    void        *base;					// base address of region
    long         ofs;					// offset into region (for locks and such)
    long         len;					// length
};

/*
 * Inter-Process Communications
 * ----------------------------
 * - lock primitives; mutexes and other semaphores
 * - shared memory
 * - message queues
 *
 * sys_ipcmkkey()
 * --------------
 * - combine proj-byte, low 16 bits of inode number, and low 8 bits of device
 *   number into a 32-bit key
 */

#if 0
typedef long      sysmtx_t;             // system mutex
typedef long      syssem_t;             // system semaphore
typedef uintptr_t sysipc_t;             // IPC object descriptor
#endif

#define IPC_CREAT       0x00000001      // create IPC object
#define IPC_EXCL        0x00000002      // fail if IPC_CREAT and object exists
#define IPC_SHARE       0x00000004      // share between kernel and user space
#define IPC_PRIVATE     0L              // special key for private object
#define IPC_STAT        1               // query IPC object attributes
#define IPC_SET         2               // set IPC object attributes
#define IPC_RMID        3               // remove IPC identifier

struct sysvkeyarg {
	char *path;							// path name for an accessible file
	int   proj;							// nonzero; the low 8 bits are used for generating a key
};

struct sysrwlock {
    long lk;    						// access lock
	long val;   						// value; may be negative
};

struct msg {
    uintptr_t qid;						// queue ID
    long      prio;						// private
    long      len;						// size of data field in bytes
    uint8_t   data[EMPTY];				// message data§
};

struct sysmq {
	struct perm  perm;					// message queue permission§
	long         lk;					// access mutex
    uintptr_t    id;					// system-wide queue ID
	struct msg  *head;					// first item in queue for this priority
	struct msg  *tail;					// last item in queue for this priority
};

/*
 * I/O interface
 * -------------
 * - different I/O objects share file-like interface
 * - I/O is buffered except for character special files and when using IO_RAW
 * - I/O descriptors are machine long-words
 *   - internally used as indices to file-structure pointer tables
 */

/* flg values for I/O operations in struct ioctl */
#define IO_RAW          0x00000001      // character I/O mode; unbuffered bytestream
#define IO_NORMAL       0x00000002      // "normal" I/O characteristics
#define IO_SEQUENTIAL   0x00000004      // object is accessed sequentially
#define IO_WILLNEED     0x00000008      // object should remain buffered
#define IO_WONTNEED     0x00000010      // object needs not be buffered
#define IO_NONBLOCK     0x00000020      // non-blocking I/O mode
#define IO_SYNC         0x00000040      // synchronous I/O mode
#define IO_NONBUF       0x00000080      // unbuffered I/O mode
/* IDEAS: IO_DIRECT */

struct syscall {
    long    arg1;
    long    arg2;
    long    arg3;
#if (LONGSIZE == 4)
    int64_t arg64;
#endif    
};

/* ioctl() */
struct ioctl {
    struct perm perm;
    off_t       ofs;
    off_t       len;
    long        flg;
};

struct select {
    fd_set          *readset;
    fd_set          *writeset;
    fd_set          *errorset;
    struct timespec *timeout;
    sigset_t        *sigmask;
};

#endif /* __KERN_SYSCALL_H__ */

