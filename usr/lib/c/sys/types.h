#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <features.h>
#include <stdint.h>
//#include <stddef.h>
#include <sys/time.h>
#if !defined(__time_types_defined)
#include <share/time.h>
#endif
#if (_BSD_SOURCE)
#include <endian.h>
//#include <sys/select.h>
#include <sys/sysmacros.h>
#endif
#include <zero/param.h>
#if !defined(_POSIX_SOURCE) && (USEBSD) && !defined(NFDBITS)
#include <kern/conf.h>
#endif
#if !defined(__struct_timeval_defined)
struct timeval {
    time_t      tv_sec;
    suseconds_t tv_usec;
};
#define __struct_timeval_defined 1
#endif

/* register-sized type */
#if (WORDSIZE == LONGSIZE)
typedef long            register_t;
#elif (WORDSIZE == LONGLONGSIZE)
typedef long long       register_t;
#endif
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
/* SysV compatibility */
typedef unsigned char   unchar;
#endif
typedef unsigned char   uchar;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned long   ulong;
#if 0
typedef int64_t         blkcnt_t;
typedef int64_t         clock_t;
#endif
typedef long            blksize_t;
typedef long            blksize_cnt;
typedef short           cnt_t;
typedef char           *caddr_t;        // core address
typedef long            daddr_t;        // disk address
typedef int32_t         dev_t;          // device number
typedef dev_t           major_t;
typedef dev_t           minor_t;
//typedef uint32_t        fsblkcnt_t;     // filesystem block count
//typedef uint32_t        fsfilcnt_t;     // filesystem file count
#if !defined(__id_t_defined)
typedef uint32_t        id_t;
#define __id_t_defined 1
#endif
#if !defined(__uid_t_defined)
typedef uint32_t        uid_t;
#define __uid_t_defined 1
#endif
typedef uint32_t        gid_t;          // group ID
typedef uintptr_t       key_t;          // IPC key
typedef unsigned long   mode_t;         // file attributes
typedef int32_t         nlink_t;        // link count
typedef int64_t         loff_t;
#if !defined(__off_t_defined)
#include <bits/off_t.h>
#endif /* !defined(__off_t_defined) */
#if (_FILE_OFFSET_BITS == 32)
typedef int32_t         blkcnt_t;
typedef uint32_t        ino_t;          // inode number
typedef uint32_t        fsblkcnt_t;
typedef uint32_t        fsfilcnt;
#else
typedef int64_t         blkcnt_t;
typedef uint64_t        ino_t;
typedef uint64_t        fsblkcnt_t;
typedef uint64_t        fsfilcnt_t;
#endif
#if !defined(__pid_t_defined)
typedef long            pid_t;          // process ID
#define __pid_t_defined 1
#endif
#if defined(_LARGEFILE64_SOURCE)
typedef uint64_t        ino64_t;
typedef int64_t         blkcnt64_t;
typedef uint64_t        fsblkcnt64_t;
typedef uint64_t        fsfilcnt64_t;
#endif
#if defined(_MSC_VER)
typedef long long       ssize_t;
#else
typedef long            ssize_t;
#endif
typedef int64_t         offset_t;
typedef uint64_t        u_offset_t;
typedef uint64_t        len_t;
typedef int64_t         diskaddr_t;
typedef unsigned long   fsid_t;

/* virtual memory related types */
typedef uintptr_t       pfn_t;          // page frame #
typedef uintptr_t       pgcnt_t;        // # of pages
typedef intptr_t        spgcnt_t;       // signed # of pages
typedef unsigned char   use_t;          // use count for swap
typedef short           sysid_t;
typedef short           index_t;
typedef void           *timeout_id_t;   // opaque handle from timeout()
typedef void           *bufcall_id_t;   // opaque handle from bufcall()

typedef long            taskid_t;
typedef long            projid_t;

#if !defined(__ctid_t_defined)
typedef long            ctid_t;
#define __ctid_t_defined 1
#endif
#if !defined(__zoneid_t_defined)
typedef long            zoneid_t;
#define __zoneid_t_defined 1
#endif

/* FIXME: <trace.h> stuff */
typedef struct {
    uint64_t flg;
}                trace_attr_t;
typedef uint64_t trace_id_t;
typedef uint64_t trace_event_id_t;
typedef uint64_t trace_event_set_t;

/* POSIX threads */
/*
 * TODO
 * ----
 * - http://pubs.opengroup.org/onlinepubs/009696699/basedefs/sys/types.h.html
 */

//#include <time.h>
#if (_BSD_SOURCE)
#include <sys/param.h>
#endif

#define P_MYPID     ((pid_t)0)
#define P_MYID      (-1)
#define NOPID       ((pid_t)-1)

#define PFN_INVALID ((pfn_t)-1)

#define NODEV       ((dev_t)-1L)

#endif /* __SYS_TYPES_H__ */

