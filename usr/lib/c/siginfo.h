#ifndef __SIGINFO_H__
#define __SIGINFO_H__

#include <signal.h>

/* si_code values */
#define SI_USER       1 // kill(), sigsend(), raise(), or abort()
#define SI_LWP        2 // _lwp_kill()
#define SI_QUEUE      3 // sigqueue()
#define SI_TIMER      4 // timer_settime()
#define SI_ASYNCIO    5 // completion of asynchronous I/O request
#define SI_MESGQ      6 // mq_notify()
/* si_code values for SIGILL */
#define ILL_ILLOPC    1 // illegal opcode
#define ILL_ILLOPN    2 // illegal operand
#define ILL_ILLADR    3 // illegal addressing mode
#define ILL_ILLTRP    4 // illegal trap
#define ILL_PRVOPC    5 // privileged opcode
#define ILL_PRVREG    6 // privileged register
#define ILL_COPROC    7 // co-processor error
#define ILL_BADSTK    8 // internal stack error
/* si_code values for SIGFPE */
#define FPE_INTDIV    1 // integer divide by zero
#define FPE_INTOVF    2 // integer overflow
#define FPE_FLTDIV    3 // floating point divide by zero
#define FPE_FLTOVF    4 // floating point overflow
#define FPE_FLTUND    5 // floating point underflow
#define FPE_FLTRES    6 // floating point inexact result
#define FPE_FLTINV    7 // invalid floating point operation
#define FPE_FLTSUB    8 // subscript out of range
/* si_code values for SIGSEGV */
#define SEGV_MAPERR   1 // address not mapped to object
#define SEGV_ACCERR   2 // invalid access permissions
/* si_code values for SIGBUS */
#define BUS_ADRALN    1 // invalid address alignment
#define BUS_ADRERR    2 // non-existent physical address
#define BUS_OBJERR    3 // object-specific hardware error
/* si_code values for SIGTRAP */
#define TRAP_BRKPT    1 // process breakpoint
#define TRAP_TRACE    2 // process trace trap
/* si_code values for SIGCHLD */
#define CLD_EXITED    1 // child has exited
#define CLD_KILLED    2 // child was killed
#define CLD_DUMPED    3 // child terminate abnormally
#define CLD_TRAPPED   4 // traced child has trapped
#define CLD_STOPPED   5 // child has stopped
#define CLD_CONTINUED 6 // stopped child has continued
/* si_code values for SIGPOLL */
#define POLL_IN       1 // input available
#define POLL_OUT      2 // output available
#define POLL_MSG      3 // input message available
#define POLL_ERR      4 // I/O error
#define POLL_PRI      5 // high-priority input available
#define POLL_HUP      6 // device disconnected
/* si_code for SIGHUP, SIGTERM (+ SIGXRES on Solaris) */
#define SI_RCTL       (~0) // resource-generated signal

#endif /* __SIGINFO_H__ */

