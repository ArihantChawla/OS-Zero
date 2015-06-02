#ifndef __KERN_SIGNAL_H__
#define __KERN_SIGNAL_H__

typedef void signalhandler_t(int sig);

/* constants. */

#define SIGNONE      0x00
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
//#define SIGUNUSED    0x1f
#define SIGRTMIN     0x20    /* minimum real-time signal */
#define SIGRTMAX     0x3f    /* maximum real-time signal */
#define SIGRTMASK    0x50    /* mask for real-time signal ID bits */
#define NSIG         64      /* # of signals */
#define NRTSIG       32      /* # of real-time signals */
#define SIGMASK      0x3f    /* mask for signal ID bits */
#define _NSIG        NSIG    /* alternative name */

#endif /*  __KERN_SIGNAL_H__ */

