#ifndef __BITS_UNISTD_H__
#define __BIT_UNISTD_H__

#include <features.h>
#include <stdint.h>

typedef int32_t uid_t;
/* types (hopefully) not declared yet... :) */
#if (_BSD_SOURCE) || (_XOPEN_SOURCE)
/* TODO: check what this type is for and what it should be :) */
typedef long socklen_t;
#endif
/* system types - TODO: figure out which headers to drop some of these into */
//typedef int32_t dev_t;
//typedef int32_t ino_t;
//typedef int32_t mode_t;
//typedef int32_t nlink_t;
//typedef int32_t gid_t;
//typedef int64_t off_t;
//typedef int64_t time_t;
//typedef int32_t blksize_t;
//typedef int64_t blkcnt_t;
/* argument bits for access() and friends */
#define R_OK 4  // check for read permission
#define W_OK 2  // check for write permission
#define X_OK 1  // check for execute permission
#define F_OK 0	// check for existence
#if (_BSD_SOURCE)
/* BSD-names for seek() and llseek() whence-argument */
#define L_SET  SEEK_SET // absolute offset
#define L_INCR SEEK_CUR // offset relative to current
#define L_XTND SEEK_END // offset relative to end of file
#endif
/* name arguments for sysconf() */
/* POSIX.1 values */
#define _SC_OS_VERSION       0x00000000
#define _SC_ARG_MAX          0x00000001		// minimum of _POSIX_ARG_MAX
#define _SC_CHILD_MAX        0x00000002     // minimum of _POSIX_CHILD_MAX
#define _SC_HOST_NAME_MAX    0x00000003     // minimum of _POSIX_LOGIN_NAME_MAX
#define _SC_LOGIN_NAME_MAX   0x00000004
#define _SC_CLK_TCK          0x00000005
#define _SC_OPEN_MAX         0x00000007
#define _SC_PAGESIZE         0x00000008
#define _SC_PAGE_SIZE        _SC_PAGESIZE
#define _SC_RE_DUP_MAX       0x00000009
#define _SC_STREAM_MAX       0x0000000a
#define _SC_SYMLOOP_MAX      0x0000000b
#define _SC_TTY_NAME_MAX     0x0000000c
#define _SC_TZNAME_MAX       0x0000000d
#define _SC_VERSION          0x0000000e
/* POSIX.2 values */
#define _SC_BC_BASE_MAX      0x0000000f
#define _SC_BC_DIM_MAX       0x00000010
#define _SC_BC_SCALE_MAX     0x00000011
#define _SC_BC_STRING_MAX    0x00000012
#define _SC_COLL_WEIGHTS_MAX 0x00000013
#define _SC_EXPR_NEST_MAX    0x00000014
#define _SC_LINE_MAX         0x00000015
//#define _SC_RE_DUP_MAX       0x00000016
#define _SC_2_VERSION        0x00000017
#define _SC_2_C_DEV          0x00000018
#define _SC2_FORT_DEV        0x00000019
#define _SC_2_LOCALEDEF      0x0000001a
#define _SC2_SW_DEV          0x0000001b
/* possibly non-standard values */
#define _SC_PHYS_PAGES       0x0000001c     // # of pages of physical memory
#define _SC_AVPHYS_PAGES     0x0000001d     // # of currently available pages of physical memory
#define _SC_NPROCESSORS_CONF 0x0000001e     // # of processors configured
#define _SC_NPROCESSORS_ONLN 0x0000001f     // # of processors currently online
#define NSYSCONF             32
#if (USEXOPENEXT) && !defined(F_LOCK)
/* these macros also appear in <fcntl.h> - keep the files consistent */
#define F_ULOCK 0
#define F_LOCK  1
#define F_TLOCK 2
#define F_TEST  3
#endif
#if defined(_GNU_SOURCE)
#define TEMP_FAILURE_RETRY(expr)                                        \
    (__extension__                                                      \
     ((long _res;                                                       \
       do {                                                             \
           _res = (long)(expr);                                         \
       } while (_res == -1L && errno == EINTR);                         \
       _res;                                                            \
         )))
#endif

#endif /* __BITS_UNISTD_H__ */

