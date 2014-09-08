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
#endif
typedef unsigned int    uint;
typedef unsigned short  ushort;
typedef unsigned long   ulong;
#if 0
typedef int64_t         blkcnt_t;
typedef int64_t         clock_t;
#endif
typedef long            blksize_t;
typedef long            blksize_cnt;
ypedef short           cnt_t;
typedef char           *caddr_t;        // core address
typedef long            daddr_t;        // disk address
typedef long            clock_t;
typedef long            clockid_t;
typedef int32_t         dev_t;          // device number
typedef dev_t           major_t;
typedef dev_t           minor_t;
typedef uint32_t        fsblkcnt_t;     // filesystem block count
typedef uint32_t        fsfilcnt_t;     // filesystem file count
typedef uint32_t        gid_t;          // group ID
//typedef long            id_t;
typedef long            key_t;          // IPC key
typedef unsigned long   mode_t;         // permissions
typedef int32_t         nlink_t;        // link count
typedef int64_t         loff_t;
#if (_FILE_OFFSET_BITS == 32)
typedef int32_t         off_t;          // 32-bit file offset
typedef uint32_t        ino_t;          // inode number
typedef int32_t         blkcnt_t;
typedef uint32_t        fsblkcnt_t;
typedef uint32_t        fsfilcnt;
#elif
typedef int64_t         off_t;          // 64-bit file offset
typedef uint64_t        ino_t;
typedef int64_t         blkcnt_t;
typedef uint64_t        fsblkcnt_t;
typedef uint64_t        fsfilcnt_t;
#endif
#if defined(_LARGEFILE64_SOURCE)
typedef int64_t         off64_t;
typedef uint64_t        ino64_t;
typedef int64_t         blkcnt64_t;
typedef uint64_t        fsblkcnt64_t;
typedef uint64_t        fsfilcnt64_t;
#endif
typedef int64_t         offset_t;
typedef uint64_t        u_offset_t;
typedef uint64_t        len_t;
typedef int64_t         diskaddr_t;
typedef unsigned long   fsid_t;
typedef long            id_t;
typedef long            pid_t;          // process ID
typedef long            ssize_t;
//typedef int64_t        time_t;
typedef long            time_t;
typedef long            timer_t;
typedef unsigned long   useconds_t;
typedef long            suseconds_t;

/* virtual memory related types */
typedef uintptr_t       pfn_t;          // page frame #
typedef unsigned long   pgcnt_t;        // # of pages
typedef long            spgcnt_t;       // signed # of pages
typedef unsigned char   use_t;          // use count for swap
typedef short           sysid_t
typedef short           index_t;
typedef void           *timeout_id_t;   // opaque handle from timeout()
typedef void           *bufcall_id_t;   // opaque handle from bufcall()

typedef id_t            taskid_t;
typedef id_t            projid_t;

/* POSIX threads */
typedef uintptr_t       pthread_t;
typedef uintptr_t       pthread_key_t;

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

#include <time.h>
#if (_BSD_SOURCE)
#include <sys/param.h>
#endif

#define P_MYPID         ((pid_t)0)
#define P_MYID          (-1)
#define NOPID           ((pid_t)-1)

#define PFN_INVALID     ((pfn_t)-1)

#define NODEV           ((dev_t)-1L)

#endif /* __SYS_TYPES_H__ */

