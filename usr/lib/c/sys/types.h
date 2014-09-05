#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <features.h>
#include <stdint.h>
#include <stddef.h>
#include <limits.h>
//#include <time.h>
#if (_BSD_SOURCE)
#include <endian.h>
#include <sys/select.h>
#include <sys/sysmacros.h>
#endif

typedef long            register_t;
typedef uint8_t         u_int8_t;
typedef uint16_t        u_int16_t;
typedef uint32_t        u_int32_t;
typedef uint64_t        u_int64_t;
#if (_BSD_SOURCE)
typedef unsigned char   u_char;
typedef unsigned short  u_short;
typedef unsigned int    u_int;
typedef unsigned long   u_long;
typedef int64_t         quad_t;
typedef uint64_t        u_quad_t;
typedef quad_t         *qaddr_t;
/* FIXME: fsid_t, daddr_t, caddr_t */
#endif
typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
/* FIXME: loff_t */
#if 0
typedef int64_t         blkcnt_t;
typedef int64_t         clock_t;
#endif
typedef int64_t         blksize_t;
typedef long            blkcnt_t;
typedef long            blksize_cnt;
typedef char           *caddr_t;        // core address
typedef int32_t         daddr_t;        // disk address
typedef long            clock_t;
typedef long            clockid_t;
typedef int32_t         dev_t;          // device number
typedef uint32_t        fsblkcnt_t;     // filesystem block count
typedef uint32_t        fsfilcnt_t;     // filesystem file count
typedef uint32_t        gid_t;          // group ID
//typedef long            id_t;
typedef int32_t         ino_t;          // inode number
typedef long            key_t;          // IPC key
typedef long            mode_t;         // permissions
typedef int32_t         nlink_t;        // link count
typedef int64_t         off_t;          // file offset
typedef long            pid_t;          // process ID
typedef long            ssize_t;
//typedef int64_t        time_t;
typedef long            time_t;
typedef long            timer_t;
typedef unsigned long   useconds_t;
typedef long            suseconds_t;

/* TODO: major(), minor(), makedev() */

#if (_POSIX_SOURCE)
#define FD_SETSIZE _POSIX_FD_SETSIZE
#else
#define FD_SETSIZE NPROCFD
#endif

typedef long       fd_mask;
#define NFDBITS    (sizeof(fd_mask) * NBBY)

#define FD_SET(fd, set)    setbit(set.fd_bits, fd)
#define FD_CLR(fd, set)    clrbit(set.fd_bits, fd)
#define FD_ISSET(fd, set)  bitset(set.fd_bits, fd)
#define FD_ZERO(set)       bzero(set.fd_bits, 0, FD_SETSIZE >> 3)
/* BSD macro? */
#define FD_COPY(dest, src) bcopy(dest, src, sizeof(*(dest)))

struct fdset {
    fd_mask fd_bits[FD_SETSIZE >> (LONGSIZELOG2 + 3)];
};
typedef struct fdset fd_set;

#if (_BSD_SOURCE)
#include <sys/param.h>
#endif

#endif /* __SYS_TYPES_H__ */

