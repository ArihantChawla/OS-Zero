#ifndef __MQUEUE_H__
#define __MQUEUE_H__

#include <stddef.h>
#include <stdint.h>
#if (_XOPEN_SOURCE >= 600) || (_POSIX_C_SOURCE >= 200112L)
#include <time.h>
#endif
#include <sys/types.h>

typedef int mqd_t;

struct mq_attr {
    long mq_flags;      // queue flags
    long mq_maxmsg;     // max # of messages
    long mq_msgsize;    // maximum message size
    long mq_curmsgs;    // # of messages in queue
    long __pad[4];
};

#if (!__KERNEL__)

extern mqd_t mq_open(const char *name, int flg, ...);
extern int   mq_close(mqd_t mq);
extern int   mq_getattr(mqd_t mq, struct mq_attr *atr);
extern int   mq_setattr(mqd_t mq, const struct mq_attr *__restrict atr,
                        struct mq_attr *__restrict oldatr);
extern int   mq_unlink(const char *name);
extern int   mq_notify(mqd_t mq, const struct sigevent *sigev);
extern int   mq_send(mqd_t mq, const char *msg, size_t len,
                     unsigned int prio);
extern ssize_t mq_receive(mqd_t mq, char *buf, size_t len,
                          unsigned int prio);
#if (_XOPEN_SOURCE >= 600) || (_POSIX_C_SOURCE >= 200112L) \
    || (USEXOPEN2K)
extern extern int   mq_timedsend(mqd_t mq, const char *msg, size_t len,
                                 unsigned int prio,
                                 const struct timespec *timeout);
extern int          mq_timedreceive(mqd_t mq, char *__restrict msg,
                                    size_t len, unsigned int prio,
                                    const struct timespec *__restrict timeout);
#endif

#endif /* !__KERNEL__ */

#endif /* __MQUEUE_H__ */

