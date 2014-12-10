#ifndef __SYS_SIGNAL_H__
#define __SYS_SIGNAL_H__

#include <bits/signal.h>
#include <sys/siginfo.h>

#define SIGNO_MASK  0x000000ff
#define SIGDEFER    0x00000100
#define SIGHOLD     0x00000200
#define SIGRELSE    0x00000400
#define SIGIGNORE   0x00000800
#define SIGPAUSE    0x00001000

#define MAXSIG      SIGRTMAX

#define S_SIGNAL    1
#define S_SIGSET    2
#define S_SIGACTION 3
#define S_NONE      4

struct sigaltstack {
	char *ss_sp;
	int   ss_size;
	int   ss_flags;
};

/* TODO: sysconf(): _SC_SIGRT_MIN, _SC_SIGRT_MAX */

#endif /* __SYS_SIGNAL_H__ */

