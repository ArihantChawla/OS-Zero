/* zero c library socket network programming interface */

#ifndef __SYS_SOCKET_H__
#define __SYS_SOCKET_H__

#include <stddef.h>
#include <stdint.h>
#include <zero/cdecl.h>
#include <zero/trix.h>

#if 0
extern struct cmsghdr *CMSG_FIRSTHDR(struct msghdr *msg);
extern struct cmsghdr *CMSG_NXTHDR(struct msghdr *msg, struct cmsghdr *cmsg);
extern size_t          CMSG_ALIGN(size_t len);
extern size_t          CMSG_SPACE(size_t len);
extern size_t          CMSG_LEN(size_t len);
extern unsigned char  *CMSG_DATA(struct cmsghdr *cmsg);
#endif

#if defined(EMPTY)
#define CMSG_DATA(cmsg) ((void *)&(cmsg)->cmsg_data)
#else
#define CMSG_DATA(cmsg) ((unsigned char *)(cmsg) + CMSG_ALIGN(sizeof(struct cmsghdr)))
#endif
#define CMSG_FIRSTHDR(msg) \
    (((msg)->msg_controllen) ? (msg)->msg_control : NULL)
#define CMSG_NEXTHDR(msg, cmsg) \
    ((cmsg) \
     ? ((((uint8_t *)(cmsg) (cmsg)->cmsg_len)
     		< (uint8_t *)(msg)->control + (msg)->controllen) \
    	? (void *)((uint8_t *)(cmsg) + CMSG_ALIGN((cmsg)->cmsg_len)) \
        : NULL) \	
     : CMSG_FIRSTHDR(msg))
#define CMSG_ALIGN(sz) \
    rounduppow2(sz, sizeof(long))
#define CMSG_LEN(sz) \
    (CMSG_ALIGN(sizeof(struct cmsghdr)) + (sz))
#define CMSG_SPACE(sz) \
    (CMSG_ALIGN(sizeof(struct cmsghdr)) + CMSG_ALIGN(sz))

typedef long          socklen_t;
typedef unsigned long sa_family_t;

struct sockaddr {
	sa_family_t sa_family;		// address family
#if defined(EMPTY)
	char        sa_data[EMPTY]; // actual address
#endif
};

struct msghdr {
	void         *msg_name;		// optional address
	socklen_t     msg_namelen;	// address length
	struct iovec *msg_iov;		// scatter-gather I/O structures
	int           msg_iovlen;	// number of scatter-gather I/O structures
	void         *msg_control;	// ancillary data
	socklen_t     msg_controllen; // ancillary data size
	int           msg_flags;    // flags
};

struct cmsghdr {
	socklen_t      cmsg_len;
	int            cmsg_level;
	int            cmsg_type;
#if defined(EMPTY)
	unsigned char  cmsg_data[EMPTY];
#endif
};

#endif /* __SYS_SOCKET_H__ */

