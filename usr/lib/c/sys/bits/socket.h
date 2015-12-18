/* zero c library socket network programming interface */

#ifndef __SYS_BITS_SOCKET_H__
#define __SYS_BITS_SOCKET_H__

#include <stddef.h>
#include <zero/cdefs.h>

/* cmsg_level argument in struct cmsghdr; level argument for set/getsockopt() */
#define SOL_SOCKET      1
/* cmsg_type with cmsg_level == SOL_SOCKET */
#define SCM_RIGHTS      1
#if (USEBSD)
#define SCM_CREDENTIALS 2
#endif
#define SCM_SECURITY    3

#define SOCK_DGRAM      1
#define SOCK_RAW        2
#define SOCK_SEQPACKET  3
#define SOCK_STREAM     4

/* name-ergument for set/getsockopt() */
#define SO_ACCEPTCONN   1
#define SO_BROADCAST    2
#define SO_DEBUG        3
#define SO_DONTROUTE    4
#define SO_ERROR        5
#define SO_KEEPALIVE    6
#define SO_LINGER       7
#define SO_OOBINLINE    8
#define SO_RCVBUF       9
#define SO_RCVLOWAT     10
#define SO_RCVTIMEO     11
#define SO_REUSEADDR    12
#define SO_SNDBUF       13
#define SO_SNDLOWAT     14
#define SO_SNDTIMEO     15
#define SO_PASSCRED     16
#define SO_PEERCRED     17
#define SO_TYPE         18

#define SOMAXCONN       128     // max bakclog queue length for listen()

/* for msg_flags field in struct msghdr */
#define MSG_CTRUNC           0x00000001 // control data truncated
#define MSG_DONTROUTE        0x00000002 // send without using routing tables
#define MSG_TRYHARD          MSG_DONTROUTE
#define MSG_EOR              0x00000004 // terminates a record (if supported)
#define MSG_OOB              0x00000008 // out-of-band data
#define MSG_PEEK             0x00000010 // leave received data in queue
#define MSG_TRUNC            0x00000020 // normal data truncated
#define MSG_WAITALL          0x00000040 // wait for complete message
#define MSG_PROBE            0x00000080 // do not send, probe path (e.g. MTU)
#define MSG_DONTWAIT         0x00000100 // nonblocking I/O
#define MSG_FIN              0x00000200
#define MSG_EOF              MSG_FIN
#define MSG_SYN              0x00000400
#define MSG_CONFIRM          0x00000800 // confirm path validity
#define MSG_RST              0x00001000
#define MSG_ERRQUEUE         0x00002000 // fetch message from error queue
#define MSG_NOSIGNAL         0x00004000 // don't generate SIGPIPE
#define MSG_MORE             0x00008000 // sender will send more
#define MSG_WAITFORONE       0x00010000 // recvmsg(): wait for one or more packets
#define MSG_SENDPAGE_NOTLAST 0x00020000 // sendpage(): not the last page
#define MSG_FASTOPEN         0x20000000 // send data in TCP SYN
#define MSG_CMSG_CLOEXEC     0x40000000

/* address families - sa_family field of struct sockaddr */
#define AF_UNSPEC      0 // unspecified
#define AF_UNIX        1 // unix domain sockets
#define AF_LOCAL       AF_UNIX // POSIX name
#define AF_INET        2 // internet domain sockets
#define AF_INET6       3

/* protocol families */
#define PF_UNSPEC      AF_UNSPEC
#define PF_UNIX        AF_UNIX
#define PF_LOCAL       AF_LOCAL
#define PF_INET        AF_INET
#define PF_INET6       AF_INET6

#define SHUT_RD        1 // disable further receive operations
#define SHUT_WR        2 // disable further send operations
#define SHUT_RDWR      3 // disable further receive and send operations

#define CMSG_ALIGN(sz) (((sz) + sizeof(long) - 1) & ~(sizeof(long)))

#if defined(EMPTY)
#define CMSG_DATA(msg) ((void *)&(msg)->cmsg_data)
#else
#define CMSG_DATA(msg) ((unsigned char *)(msg) + CMSG_ALIGN(sizeof(struct cmsghdr)))
#endif
#define CMSG_FIRSTHDR(msg)                                              \
    (((msg)->msg_controllen) ? (msg)->msg_control : NULL)
#define CMSG_NXTHDR(msg, cmsg)                                          \
    (((msg)->cmsg_len >= sizeof(struct cmsghdr))                        \
     ? (((unsigned char *)(msg) + CMSG_ALIGN((msg)->cmsg_len)           \
         < (unsigned char *)(msg)->msg_control + (msg)->controllen)     \
        ? (struct cmsghdr *)((unsigned char *)(msg)                     \
                             + CMSG_ALIGN((msg)->cmsg_len))             \
        : NULL)                                                         \
     : NULL)
#define CMSG_LEN(sz)                                                    \
    (CMSG_ALIGN(sizeof(struct cmsghdr)) + (sz))
#define CMSG_SPACE(sz)                                                  \
    (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(sz))

#endif /* __SYS_BITS_SOCKET_H__ */

