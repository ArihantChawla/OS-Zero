#ifndef __SYS_SIGINFO_H__
#define __SYS_SIGINFO_H__

#include <sys/types.h>

typedef struct {
    int           si_signo;
    int           si_code;
    int           si_errno;
    pid_t         si_pid;
    uid_t         si_uid;
    void         *si_addr;
    int           si_status;
    long          si_band;
    union sigval  si_value;
};

#endif /* __SYS_SIGINFO_H__ */

