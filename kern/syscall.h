#ifndef __KERN_SYSCALL_H__
#define __KERN_SYSCALL_H__

#include <features.h>
#include <stdint.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <zero/cdefs.h>
#include <mach/param.h>
#include <kern/syscallnum.h>
#include <kern/perm.h>
#include <kern/signal.h>
#include <kern/mem/obj.h>
#include <kern/obj.h>

/* TODO
 * ----
 * - sysreg_t sys_prof(sysreg_t cmd, sysreg_t pid, sysreg_t parm);
 *   - cmd
 *     - one of PROFON, PROFOFF
 *   - pid:
 *     - -1 - all processes (of system if not root)
 *     - -X - process group of X;
 *     - 0  - the kernel [subsystems]
 *     - X (posivite) - process X
 *   - parm - bitmask
 *     - PROFLOG
 *     - PROFSUMMARY
 *     - PROFHIRES (nanoseconds, if 0 then microseconds)
 * - sysreg_t sys_trace(sysreg_t cmd, sysreg_t pid, sysreg_t parm);
 *   - cmd
 *     - one of TRACEON, TRACEOFF
 *   - pid
 *     - as for sys_prof()
 *   - parm - bitmask
 *     - TRACELOG
 *     - TRACESUMMARY
 *     - TRACETRAP
 *     - TRACEMEM
 *     - TRACEIO
 * - sysreg_t sys_chroot(char *path);
 * - sysreg_t sys_jailctl(sysreg_t cmd, sysreg_t pid, void *parm);
 *   - cmd
 *     - JAILON, JAILOFF, ...
 *   - pid
 *     - -1 - all processes (of user if not root)
 *     - -X - process group of X
 *     - 0 - self (+ children)
 *     - +X - process ID X
 *   - parm; see struct jailparm below
 * - sysreg_t sys_ioperm(sysreg_t cmd, sysreg_t pid, uintptr_t parm);
 *   - cmd - IOENABLE, IODISABLE, IOCHK, IOSETMAP, IOMAP, IOUNMAP
 *   - pid - as for sys_jailctl()
 *   - parm
 *     - IOENABLE, IODISABLE, IOCHKPERM: parm is port number
 *     - IOSETMAP: parm is I/O-permission bitmap for ports
 *     - IOMAP, IOUNMAP: parm is memory-mapped device I/O address
 */

#if 0
struct jailparm {
    /* memory attributes */
    size_t heapsize;
    size_t mapsize;
    size_t virtsize;
};
#endif

extern void ksyscall(void);

typedef intptr_t sysreg_t;

#define SYS_NODESC (-1)
#define SYS_NOARG  (-1)

/*
 * TODO
 * ----
 * sysctl functionality
 * --------------------
 * - commands to access system clock
 * - commands to query system status
 * - commands to query and set system attributes
 */

/*
 * system call interface
 * ---------------------
 * - networking/socket support shall be added after the system is implemented
 *   for simple desktop use
 *
 * process interface
 * -----------------
 * sysreg_t sys_sysctl(sysreg_t cmd, sysreg_t parm, void *arg);
 * - perform system control actions
 * sysreg_t sys_exit(sysreg_t val, sysreg_t flg);
 * - exit process normally
 * void sys_abort(void);
 * - exit problem abnormally
 * sysreg_t sys_fork(sysreg_t flg);
 * - create new process by cloning current one
 * sysreg_t sys_exec(char *path, sysreg_t parm, void *arg);
 * - execute program image
 * sysreg_t sys_throp(sysreg_t cmd, sysreg_t parm, void *arg);
 * - perform thread control operations
 * sysreg_t sys_pctl(sysreg_t cmd, sysreg_t parm, void *arg);
 * - perform process control operations
 * sysreg_t sys_sigop(sysreg_t cmd, sysreg_t parm, void *arg);
 * - perform signal actions
 *
 * memory interface
 * ----------------
 * sysreg_t  sys_brk(void *adr);
 * - set top of heap to adr
 * void *sys_map(sysreg_t desc, sysreg_t flg, struct memreg *arg);
 * - map file or anonymous memory
 * sysreg_t  sys_umap(void *adr, size_t size);
 * - unmap file or anonymous memory
 * sysreg_t  sys_mhint(long cmd, sysreg_t flg, struct memreg *arg);
 * - hint kernel about memory region use characteristics
 * sysreg_t  sys_mctl(long cmd, sysreg_t flg, struct memreg *arg);
 * - memory control operations; locks, permissions, etc.
 *
 * IPC
 * ---
 * key_t sys_mkipckey(void *arg);
 * key_t sys_rmipckey(key_t key);
 *
 * shared memory
 * -------------
 * uintptr_t  sys_shmget(sysreg_t key, size_t size, sysreg_t flg);
 * - look up shared memory segment associated with key or create a new one with key IPC_PRIVATE
 * void      *sys_shmat(uintptr_t id, void *adr, sysreg_t flg);
 * - map shared memory segment to virtual address space of calling process; specifying adr is not
 *   guaranteed to succeed
 * sysreg_t       sys_shmdt(void *adr);
 * - unmap shared memory segment from virtual address space of calling process
 * sysreg_t       sys_shmctl(uintptr_t id, sysreg_t cmd, void *arg);
 * - perform shared memory control actions
 *
 * semaphores
 * ----------
 * uintptr_t sys_mksem(sysreg_t cnt);
 * - allocate semaphore and initialise it with value cnt
 * void      sys_rmsem(uintptr_t id);
 * - deallocate semaphore
 * sysreg_t  sys_semwait(uintptr_t sem, sysreg_t n);
 * - decrease semaphore value by n
 * sysreg_t  sys_semsignal(uintptr_t sem, sysreg_t n);
 * - attempt to increase semaphore value by n
 *
 * read-write locks
 * ----------------
 * FIXME
 * -----
 * uintptr_t sys_rwlkinit(sysreg_t desc, sysreg_t flg, struct objreg *arg);
 * - allocate and initialise read-write lock
 * void      sys_rwlkrm(sysreg_t desc, sysreg_t flg);
 * - deallocate read-write lock
 * uintptr_t    sys_rdlock(sysreg_t desc, sysreg_t flg, struct objreg *arg);
 * - acquire read-lock of object desc
 * uintptr_t sys_wrlock(sysreg_t desc, sysreg_t flg, struct objreg *arg);
 * - acquire write-lock on object describe
 * void      sys_unlock(uintptr_t lk);
 * - unlock a read-write lock
 *
 * message queues
 * --------------
 * FIXME (sys_mqpeek()?)
 * -----
 * uintptr_t sys_mqinit(sysreg_t mq, sysreg_t nprio, size_t qsize);
 * sysreg_t      sys_mqsend(sysreg_t mq, sysreg_t prio, struct msg *arg);
 * sysreg_t      sys_mqregv(sysreg_t mq, sysreg_t prio, struct msg *arg);
 *
 * events
 * ------
 * TODO: evreg(), evsend(), evrecv(), evctl()
 *
 * I/O interface
 * -------------
 * sysreg_t sys_mnt(char *path1, char *path2, void *arg);
 * - mount filesystem path2 to mountpoint path1
 * sysreg_t sys_umnt(char *path, sysreg_t flg);
 * - unmount filesystem mounted to path§
 * sysreg_t sys_readdir(sysreg_t desc, struct dirent *dirent, sysreg_t cnt);
 * - read directory contents
 * sysreg_t sys_open(char *path, sysreg_t flg, sysreg_t mode);
 * - open file path for operations as selected in mode§
 * sysreg_t sys_trunc(char *path, off_t len);
 * - truncate file path to length len; toss contents behind len
 * sysreg_t sys_close(sysreg_t desc);
 * - close file refered to by desc
 * sysreg_t sys_read(sysreg_t desc, void *buf, size_t len);
 * - read next maximum of len data bytes from file desc into buf
 * sysreg_t sys_readv(sysreg_t desc, sysreg_t narg, void *arg);
 * - read from several files into a buffer described by arg
 * sysreg_t sys_write(sysreg_t desc, void *buf, size_t len);
 * - attempt to write len data bytes from memory pointed to by buf to file desc
 * sysreg_t sys_writev(sysreg_t desc, sysreg_t narg, void *arg);
 * - write into file desc from several buffers described by arg
 * sysreg_t sys_seek(sysreg_t desc, off_t ofs, sysreg_t whence);
 * - set seek position of file desc
 * sysreg_t sys_falloc(sysreg_t desc, sysreg_t parm, size_t len);
 * - attempt to preallocate len bytes of device space for file desc
 * sysreg_t sys_stat(char *path, struct stat *buf, sysreg_t flg);
 * - query file attributes
 * sysreg_t sys_fhint(sysreg_t desc, sysreg_t flg, struct objreg *arg);
 * - hint kernel of file usage patterns
 * sysreg_t sys_ioctl(sysreg_t desc, sysreg_t cmd, void *arg);
 * - perform device-dependent I/O operations
 * sysreg_t sys_fctl(sysreg_t desc, sysreg_t cmd, void *arg);
 * - perform file control operations
 * sysreg_t sys_poll(struct pollfd *fds, sysreg_t nfd, sysreg_t timeout);
 * sysreg_t sys_select(sysreg_t nfds, struct select *arg);
 * sysreg_t sys_epoll(sysreg_t desc, sysreg_t cmd, void *arg);
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
#define SYSCTL_HALT     0x01U   // halt() and reboot()
#define SYSCTL_SYSINFO  0x02U   // sysinfo()
#define SYSCTL_SYSCONF  0x03U   // sysconf(); probe system parameters
#define SYSCTL_SYSSTAT  0x04U   // query system statistics; getrusage()
#define SYSCTL_TIME     0x05U   // system clock access
#define SYSCTL_PROF     0x06U   // profile system execution
#define SYSCTL_TRACE    0x07U   // trace system execution
#define SYSCTL_CHROOT   0x08U   // change process root directory
#define SYSCTL_JAIL     0x09U   // set jail parameters

/* SYSCTL_TRACE */
#define SYSCTL_TRACEON  0x00U
#define SYSCTL_TRACEOFF 0x01U
/* SYSCTL_PROF */
#define SYSCTL_PROFON  0x00U
#define SYSCTL_PROFOFF 0x01U

/* parm */
/* flg value for SYSCTL_HALT */
#define SYSCTL_REBOOT   0x01U   // reboot()
#define SYSCTL_HIBER    0x02U   // hibernate, i.e. save software state
/* SYSCTL_SYSINFO */
#define SYSCTL_DTABSIZE 0x01U   // query process descriptor table size; getdtablesize()
#define SYSCTL_STKSIZE  0x02U   // query process stack size; getstksize()
#define SYSCTL_RAMSIZE  0x03U   // query system memory size; getramsize()
#define SYSCTL_VMEMSIZE 0x04U   // query virtual memory size; getvirtsize()
#define SYSCTL_NPROC    0x05U   // query or set maximum number of processes on system
#define SYSCTL_NTHR     0x06U   // query or set maximum number of threads on system
/* SYSCTL_SYSCONF */
/* POSIX-functionality */
#define SYSCTL_PAGES    0x01U
#define SYSCTL_AVPAGES  0x02U
#define SYSCTL_NUMPROC  0x03U
#define SYSCTL_PROCON   0x04U
/* Zero extensions */
#define SYSCTL_BLKSIZE  0x05U   // buffer cache size for block I/O (BUFSIZE)
#if 0 /* user-space */
#define SYSCTL_L1SIZE   0x05U   // L1 cache size
#define SYSCTL_L2SIZE   0x06U   // L2 cache size
#endif
/* SYSCTL_SYSSTAT */
#define SYSCTL_UPTIME   0x01U   // query system uptime
#define SYSCTL_LOAD     0x02U   // query load averages
/* SYSCTL_TIME */
#define SYSCTL_GETTIME  0x01U   // query system time
#define SYSCTL_SETTIME  0x02U   // set system time
#define SYSCTL_GETTZONE 0x03U   // get timezone
#define SYSCTL_SETTZONE 0x04U   // set timezone

/* exit() parm flags */
#define EXIT_DEF        0x00U
#define EXIT_DUMPACCT   0x01U   // dump system information at process exit

/* abort() parm flags */
#define ABORT_DUMPCORE  0x01U   // dump core image at abnormal process exit

/* fork() parm flags */
#define FORK_VFORK      0x01U   // vfork() semantics; share address space with parent
#define FORK_COW        0x02U   // copy on write optimisations

struct sysprofarg {
    long state;
    long pid;
    long parm;
};

struct systracearg {
    long state;
    long pid;
    long parm;
};

struct sysioperm {
    long      cmd;
    long      pid;
    uintptr_t parm;
};

/* thread interface */

/* TODO: MTX, SEM, and COND operations under IPC */

/* throp() commands */
#define THR_YIELD           1
#define THR_NEW             2      // pthread_create()
#define THR_JOIN            3      // pthread_join()
#define THR_DETACH          4      // pthread_detach()
#define THR_EXIT            5      // pthread_exit()
#define THR_CLEANUP         6      // cleanup; pop and execute handlers etc.
#define THR_KEYOP           7      // create, delete
#define THR_SYSOP           8      // atfork, sigmask, sched, scope
#define THR_STKOP           9      // stack; adr, size, guardsize
#define THR_RTOP           10      // realtime thread settings
#define THR_SETATR         11      // set other attributes

/* pctl() commands */
#define PROC_GETPID        0x01    // getpid()
#define PROC_GETPGRP       0x02    // getpgrp()
#define PROC_SETSID        0x03    // setsid()
#define PROC_WAIT          0x04    // wait()
#define PROC_USLEEP        0x05    // usleep()
#define PROC_NANOSLEEP     0x06    // nanosleep()
#define PROC_SETSCHED      0x07    // nice(), scheduler classes, ...
#define PROC_STAT          0x08    // getrusage()
#define PROC_GETLIM        0x09    // getrlimit(), getstksize(), getdtablesize()
#define PROC_SETLIM        0x0a    // setrlimit()
#define PROC_ATEXIT        0x0b    // atexit(), on_exit()
#define PROC_SETTMR        0x0c    // set timer up; interval, one-shot, alarm
#define PROC_SETPERM       0x0d    // umask()
#define PROC_GETPERM       0x0e    // chmod(), fchmodat()
#define PROC_MAPBUF        0x0f    // mapbuf()
/* pctl() parm attributes */
/* PROC_GETLIM, PROC_SETLIM */
#define PROC_NTHR          0x01 // max # of threads for process
#define PROC_STKSIZE       0x02	// process stack size
#define PROC_KERNSTKSIZE   0x03 // process kernel stack size
#define PROC_DESCTABSIZE   0x04 // process descriptor table size
/* PROC_MAPBUF */
#define PROC_LFBUF         0x00
#define PROC_KBDBUF        0x01 // keyboard input buffer
#define PROC_MOUSEBUF      0x02 // mouse input buffer

/* argument structure for pctl() with PROC_GETLIM and PROC_SETLIM */
struct sysproclim {
    unsigned long soft;
    unsigned long hard;
};

#if 0
/* pctl() parameters */
/* PROC_WAIT flags */
#define PROC_WAITPID 0x01       // wait for pid
#define PROC_WAITCLD 0x02       // wait for children in the group pid
#define PROC_WAITGRP 0x04       // wait for children in the group of caller
#define PROC_WAITANY 0x08       // wait for any child process
#endif

struct syswait {
    long  pid;  // processes to wait for
    long *stat; // storage for exit status
    void *data; // rusage etc.
};

/* signal interface */

/* sigop() commands */
#define SIG_WAIT      0x01      // pause()
#define SIG_SETFUNC   0x02      // signal()/sigaction()
#define SIG_PROCMASK  0x03      // sigprocmask()
#define SIG_SEND      0x04      // raise(), kill(), etc.
#define SIG_SETSTK    0x05      // sigaltstack(), sigstack()
#define SIG_SUSPEND   0x06      // sigsuspend(), sigpause()
/* sigop() parm-argument bits */
/* sigaction() definitions */
#define SIG_NOCLDSTOP  0x00000001
#define SIG_NOCLDWAIT  0x00000002
#define SIG_NODEFER    0x00000004
#define SIG_RESETHAND  0x00000008
#define SIG_SIGINFO    0x00000010
#define SIG_TRAMPOLINE 0x80000000
/* non-POSIX */
#define SIG_ONSTACK    0x80000000
#define SIG_RESTART    0x40000000
#define SIG_FASTINTR   0x20000000
#define SIG_NOSTACK    0x10000000
/* values for struct syssigarg */
/* pid values */
#define SIG_SELF       (-0x01)  // raise()
#define SIG_CLD        (-0x02)  // send to children
#define SIG_GRP        (-0x03)  // send to group
#define SIG_PROPCHLD   0x01     // propagate to child processes
#define SIG_PROPGRP    0x02     // propagate to process group
/* SIG_PAUSE */
#define SIG_EXIT       0x01     // exit process on signal
#define SIG_DUMP       0x02     // dump core on signal

struct syssigarg {
    pid_t  pid;
    long   sig;
    long   flg;
    void  *func;
};

/* memory interface */

/* sys_map() */
#define MAP_PRIVATE       0x00000000    // changes are private - default
#define MAP_FILE          0x00000001    // mapped from file or device
#define MAP_ANON          0x00000002    // allocate memory, swap space, zeroed
#define MAP_ANONYMOUS     MAP_ANON
#define MAP_TYPE          0x0000000f    // type field mask
#define MAP_COPY          0x00000010    // "copy" region at mmap time - WTF? :)
#define MAP_SHARED        0x00000020    // share changes
#define MAP_FIXED         0x00000040    // must use requested address
#define MAP_NOEXTEND      0x00000080    // for MAP_FILE, don't change file size
#define MAP_HASSEMAPHORE  0x00000100    // region may have semaphores
#define MAP_HASSEMPHORE   MAP_HASSEMAPHORE
#define MAP_GROWSDOWN     0x00000200    // mapping extends downward (stacks)
#define MAP_LOCKED        0x00000400    // lock pages a'la mlock()
#define MAP_HUGETLB       0x00000400    // map with huge pages
#define MAP_NONBLOCK      0x00000800    // don't block on I/O
#define MAP_POPULATE      0x00001000    // prefault page tables
#define MAP_STACK         0x00002000
#define MAP_UNINITIALIZED 0x00004000    // don't zero; SECURITY! (ignored)
#define MAP_SINGLE        0x00008000    // kernel + single user process
/* sys_map() and sys_mhint() */
#define MEM_NORMAL        1             // no special treatment
#define MEM_RANDOM        2             // expect "random" references
#define MEM_SEQUENTIAL    3             // expect sequential references
#define MEM_WILLNEED      4             // will need the region
#define MEM_DONTNEED      5             // won't need the region
#define MEM_REMOVE        6
#define MEM_DONTFORK      7
#define MEM_DOFORK        8
#define MEM_DONTDUMP      9
#define MEM_DODUMP        10
/* sys_mctl() */
/* cmd */
#define MEM_SHMAP         0x01          // shmap()
#define MEM_LOCK          0x02          // mlock(), mlockall()
#define MEM_UNLOCK        0x03          // munlock(), munlockall()
/* REFERENCE: <kern/perm.h> */
#define MEM_GETPERM       0x04
#define MEM_SETPERM       0x05
/* flg-argument bits for MEM_SHMAP */
#define MEM_DUALMAP       0x00000001
#if 0
struct sysmemarg {
    long          cmd;  // memory command
    struct memreg reg;
};
#endif

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

struct sysvkeyarg {
	char *path;                     // path name for an accessible file
	int   proj;		        // nonzero; the low 8 bits are used for generating a key
};

struct sysrwlock {
    long lk;    			// access lock
	long val;   			// value; may be negative
};

struct sysmsg {
    uintptr_t   qid;			// queue ID
    long        prio;			// private
    long        len;			// size of data field in bytes
	struct msg *prev;
	struct msg *next;
    uint8_t     data[EMPTY];		// message data§
};

struct sysmq {
	long         lk;		// access mutex
	struct perm  perm;		// message queue permission§
    uintptr_t    id;			// system-wide queue ID
	struct msg  *head;		// first item in queue for this priority
	struct msg  *tail;		// last item in queue for this priority
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
#define IO_RAW        0x00000001        // character I/O unbuffered bytestream
#define IO_NORMAL     0x00000002        // "normal" I/O characteristics
#define IO_SEQUENTIAL 0x00000004        // object is accessed sequentially
#define IO_WILLNEED   0x00000008        // object should remain buffered
#define IO_WONTNEED   0x00000010        // object needs not be buffered
#define IO_NONBLOCK   0x00000020        // non-blocking I/O mode
#define IO_SYNC       0x00000040        // synchronous I/O mode
#define IO_NONBUF     0x00000080        // unbuffered I/O mode
/* IDEAS: IO_DIRECT */

struct sysop {
    long     num;                       // system call ID
    sysreg_t arg1;                      // first argument
    sysreg_t arg2;                      // second argument
    sysreg_t arg3;                      // third argument
#if (LONGSIZE == 4)
    int64_t  arg64;                     // 64-bit argument
#endif
};

/* structure for restarting interrupted system calls */
struct sysctx {
    long     num;       // current system call number
    /* updated arguments for restart */
    sysreg_t arg1;
    sysreg_t arg2;
    sysreg_t arg3;
    sysreg_t arg4;
    sysreg_t arg5;
    sysreg_t arg6;
#if 0
    sysreg_t ret;
    int      errnum;    // error status (for errno)
#endif
};

struct sysioreg {
    struct perm perm;		        // permission structure
    off_t       ofs;
    off_t       len;
};

#define SYS_IOCTL_IOPERM 0x01           // need CAP_SYS_RAWIO

/* ioctl() */
struct sysioctl {
//	long            cmd;            // command to be executed
    long            parm;               // command parameters such as flag-bits
    struct sysioreg reg;
};

struct sysselect {
    fd_set          *readset;
    fd_set          *writeset;
    fd_set          *errorset;
    struct timespec *timeout;
    sigset_t        *sigmask;
};

#endif /* __KERN_SYSCALL_H__ */

