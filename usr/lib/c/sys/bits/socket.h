/* zero c library socket network programming interface */

#ifndef __SYS_BITS_SOCKET_H__
#define __SYS_BITS_SOCKET_H__

#include <stddef.h>
#include <zero/cdefs.h>

/* cmsg_level argument in struct cmsghdr; level argument for set/getsockopt() */
#define SOL_SOCKET      (~0)
/* cmsg_type with cmsg_level == SOL_SOCKET */
#define SCM_RIGHTS      1
#if (USEBSD)
#define SCM_CREDENTIALS 2
#endif
#define SCM_SECURITY    3

#define SOCK_STREAM     1       // stream socket
#define SOCK_DGRAM      2       // datagram socket
#define SOCK_RAW        3       // raw-protocol interface
#define SOCK_RDM        4       // reliably-delivered message
#define SOCK_SEQPACKET  5       // sequenced packet stream

/* name-ergument for set/getsockopt() */
#define SO_ERROR        (1 << 0)
#define SO_DEBUG        (1 << 1)
#define SO_ACCEPTCONN   (1 << 2)
#define SO_REUSEADDR    (1 << 3)
#define SO_REUSEPORT    (1 << 4)
#define SO_KEEPALIVE    (1 << 5)
#define SO_DONTROUTE    (1 << 6)
#define SO_BROADCAST    (1 << 7)
#define SO_USELOOPBACK  (1 << 8)
#define SO_LINGER       (1 << 9)
#define SO_OOBINLINE    (1 << 10)
#define SO_RCVBUF       (1 << 11)
#define SO_SNDBUF       (1 << 12)
#define SO_RCVLOWAT     (1 << 13)
#define SO_RCVTIMEO     (1 << 14)
#define SO_SNDLOWAT     (1 << 15)
#define SO_SNDTIMEO     (1 << 16)
#define SO_PASSCRED     (1 << 17)
#define SO_PEERCRED     (1 << 18)
#define SO_TYPE         (1 << 19)
#define SO_BINDTODEVICE (1 << 20)
#define SO_NONBLOCK     (1 << 21)

#define SOMAXCONN       32      // max backlog queue length for listen()

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
#define MSG_EOF              MSG_FIN    // data completes connection
#define MSG_SYN              0x00000400
#define MSG_CONFIRM          0x00000800 // confirm path validity
#define MSG_RST              0x00001000
#define MSG_ERRQUEUE         0x00002000 // fetch message from error queue
#define MSG_NOSIGNAL         0x00004000 // don't generate SIGPIPE
#define MSG_MORE             0x00008000 // sender will send more
#define MSG_WAITFORONE       0x00010000 // recvmsg(): wait for packets
#define MSG_SENDPAGE_NOTLAST 0x00020000 // sendpage(): not the last page
#define MSG_BCAST            0x00040000 // broadcast packet
#define MSG_MCAST            0x00080000 // multicast packet
#define MSG_FASTOPEN         0x20000000 // send data in TCP SYN
#define MSG_CMSG_CLOEXEC     0x40000000

#define __saisfamily(af)     ((af) < AF_NFAMILY)
/* address families - sa_family field of struct sockaddr */
#define AF_UNSPEC      0 // unspecified
#define AF_RAW         1 // raw socket
#define AF_LINK        2
#define AF_ROUTE       3
#define AF_UNIX        4 // unix domain sockets
#define AF_LOCAL       AF_UNIX // POSIX name
#define AF_INET        5 // internet domain sockets
#define AF_INET6       6
#define AF_ISO         7
#define AF_OSI         AF_ISO
#define AF_BLUETOOTH   8
#define AF_NFAMILY     9

/* protocol families */
#define PF_UNSPEC      AF_UNSPEC
#define PF_RAW         AF_RAW
#define PF_LINK        AF_LINK
#define PF_ROUTE       AF_ROUTE
#define PF_UNIX        AF_UNIX
#define PF_LOCAL       AF_LOCAL
#define PF_INET        AF_INET
#define PF_INET6       AF_INET6
#define PF_ISO         AF_ISO
#define PF_OSI         AF_OSI
#define PF_BLUETOOTH   AF_BLUETOOTH

#define SHUT_RD        0 // disable further receive operations
#define SHUT_WR        1 // disable further send operations
#define SHUT_RDWR      2 // disable further receive and send operations

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

