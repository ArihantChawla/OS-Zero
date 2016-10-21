/*
 * REFERENCE
 * ---------
 * http://pubs.opengroup.org/onlinepubs/009695399/basedefs/signal.h.html
 */

#ifndef __KERN_SIGNAL_H__
#define __KERN_SIGNAL_H__

/*
 * from Open Group
 * ---------------
 * - the following signals are supported on all implementations
 *
 * signal       default action  description
 * ------       --------------  -----------
 * SIGABRT      2               process abort signal
 * SIGALRM      1               alarm clock
 * SIGFPE       2               erroneous arithmetic operation
 * SIGHUP       1               hangup
 * SIGILL       2               illegal instruction
 * SIGINT       1               terminal interrupt signal
 * SIGKILL 	1        	kill (cannot be caught or ignored).
 * SIGPIPE 	1               write on a pipe with no one to read it.
 * SIGQUIT 	2               terminal quit signal.
 * SIGSEGV 	2               invalid memory reference.
 * SIGTERM 	1               termination signal.
 * SIGUSR1 	1               user-defined signal 1.
 * SIGUSR2 	1               user-defined signal 2.
 * SIGCHLD 	3               child process terminated or stopped.
 * SIGCONT 	5               continue executing, if stopped.
 * SIGSTOP 	4               stop executing (cannot be caught or ignored).
 * SIGTSTP 	4               terminal stop signal.
 * SIGTTIN 	4               background process attempting read.
 * SIGTTOU 	4               background process attempting write.
 * SIGBUS 	2               access to undefined portion of a memory object.
 * SIGPOLL 	1               pollable event.
 * SIGPROF 	1               profiling timer expired.
 * SIGSYS 	2               bad system call.
 * SIGTRAP 	2               trace/breakpoint trap.
 * SIGURG 	3               high bandwidth data is available at a socket.
 * SIGVTALRM 	1               virtual timer expired.
 * SIGXCPU 	2               CPU time limit exceeded.
 * SIGXFSZ 	2               file size limit exceeded.
 *
 * default actions
 * ---------------
 * 
 * 1
 *
 * Abnormal termination of the process. The process is terminated with all the
 * consequences of _exit() except that the status made available to wait() and
 * waitpid() indicates abnormal termination by the specified signal. 
 * 
 * 2
 *
 * Abnormal termination of the process. Additionally, implementation-dependent
 * abnormal termination actions, such as creation of a core file, may occur. 
 *
 * 3
 *
 * Ignore the signal. 
 *
 * 4
 *
 * Stop the process. 
 *
 * 5
 *
 * Continue the process, if it is stopped; otherwise ignore the signal. 
 */

/* constants; signal names */

#define SIGNONE      0x00    /* no signal */
#define SIGCLD       0x00    /* UNSUPPORTED - System V, NOT SIGCHLD semantics */
#define SIGEMT       0x00    /* UNSUPPORTED */
#define SIGHUP       0x01    /* POSIX */
#define SIGINT       0x02    /* ISO C */
#define SIGQUIT      0x03    /* POSIX */
#define SIGILL       0x04    /* ISO C */
#define SIGTRAP      0x05    /* Unix */
#define SIGABRT      0x06    /* ISO C */
#define SIGIOT       SIGABRT /* 4.2BSD */
#define SIGUSR1      0x07    /* POSIX */
#define SIGDEBUG     SIGUSR1 /* debugging */
#define SIGFPE       0x08    /* ISO C */
#define SIGKILL      0x09    /* POSIX */
#define SIGBUS       0x0a    /* 4.2BSD */
#define SIGSEGV      0x0b    /* ISO C */
#define SIGUSR2      0x0c    /* POSIX */
#define SIGPIPE      0x0d    /* POSIX */
#define SIGALRM      0x0e    /* POSIX */
#define SIGTERM      0x0f    /* ISO C */
#define SIGSTKFLT    0x10
#define SIGCHLD      0x11    /* POSIX */
#define SIGCONT      0x12    /* POSIX */
#define SIGSTOP      0x13    /* POSIX */
#define SIGTSTP      0x14    /* POSIX */
#define SIGTTIN      0x15    /* POSIX */
#define SIGTTOU      0x16    /* POSIX */
#define SIGURG       0x17    /* 4.2BSD */
#define SIGXCPU      0x18    /* 4.2BSD */
#define SIGXFSZ      0x19    /* 4.2BSD */
#define SIGVTALRM    0x1a    /* 4.2BSD */
#define SIGPROF      0x1b    /* 4.2BSD */
#define SIGWINCH     0x1c    /* 4.3BSD, Sun */
#define SIGIO        0x1d    /* 4.2BSD */
#define SIGPOLL      SIGIO   /* System V */
#define SIGPWR       0x1e    /* System V */
#define SIGINFO      SIGPWR
#define SIGSYS       0x1f
#define SIGRTMIN     0x20    /* minimum real-time signal */
#define SIGRTMAX     0x3f    /* maximum real-time signal */
#define _SIGRTBIT    0x20    /* bit set for all real-time signals */
#define NSIG         64      /* # of signals */
#define NRTSIG       32      /* # of real-time signals */
#define _SIGMASK     0x3f    /* mask for signal ID bits */
#define _NSIG        NSIG    /* alternative name */
#define RTSIG_MAX    NRTSIG

#endif /*  __KERN_SIGNAL_H__ */

