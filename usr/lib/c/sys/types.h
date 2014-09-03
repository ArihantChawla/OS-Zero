#ifndef __SYS_TYPES_H__
#define __SYS_TYPES_H__

#include <features.h>
#include <stdint.h>
#include <stddef.h>
//#include <time.h>
#if (_BSD_SOURCE)
#include <endian.h>
#include <sys/select.h>
#include <sys/sysmacros.h>
#endif

typedef long           register_t;
typedef uint8_t        u_int8_t;
typedef uint16_t       u_int16_t;
typedef uint32_t       u_int32_t;
typedef uint64_t       u_int64_t;
#if (_BSD_SOURCE)
typedef unsigned char  u_char;
typedef unsigned short u_short;
typedef unsigned int   u_int;
typedef unsigned long  u_long;
typedef int64_t        quad_t;
/* FIXME: fsid_t, daddr_t, caddr_t */
typedef uint64_t       u_quad_t;
#endif
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned long  ulong;
/* FIXME: loff_t */
typedef int64_t        blkcnt_t;
typedef int64_t        blksize_t;
typedef int64_t        clock_t;
typedef long           clockid_t;
typedef int64_t        dev_t;
typedef uint32_t       fsblkcnt_t;
typedef uint32_t       fsfilcnt_t;
typedef uint32_t       gid_t;
typedef long           id_t;
typedef int32_t        ino_t;
typedef long           key_t;
typedef long           mode_t;
typedef int32_t        nlink_t;
typedef int64_t        off_t;
typedef long           pid_t;
typedef long           ssize_t;
typedef int64_t        time_t;
typedef long           timer_t;
typedef unsigned long  useconds_t;
typedef long           seconds_t;

#endif /* __SYS_TYPES_H__ */

