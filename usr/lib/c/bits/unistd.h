#ifndef __BITS_UNISTD_H__
#define __BITS_UNISTD_H__

#include <features.h>
#include <stdint.h>

//typedef int32_t uid_t;
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

/* name-arguments for sysconf() */
/* POSIX.1 values */
#define _SC_OS_VERSION       0x00000000 // OS [kernel] version
#define _SC_VERSION          0x00000001 // POSIX version
#define _SC_ARG_MAX          0x00000002 // minimum of _POSIX_ARG_MAX
#define _SC_CHILD_MAX        0x00000003 // minimum of _POSIX_CHILD_MAX
#define _SC_HOST_NAME_MAX    0x00000004 // minimum of _POSIX_LOGIN_NAME_MAX
#define _SC_LOGIN_NAME_MAX   0x00000005 // # of bytes in login name
#define _SC_CLK_TCK          0x00000006 // # of ticks per second
#define _SC_OPEN_MAX         0x00000007 // # of open files
#define _SC_PAGESIZE         0x00000008 // page size in bytes
#define _SC_PAGE_SIZE        _SC_PAGESIZE // synonymous with _SC_PAGESIZE
#define _SC_RE_DUP_MAX       0x00000009 // regexec(), regcomp()
#define _SC_STREAM_MAX       0x0000000a // FOPEN_MAX
#define _SC_SYMLOOP_MAX      0x0000000b // max # of symlinks in pathnames
#define _SC_TTY_NAME_MAX     0x0000000c // # of bytes in a terminal device name
#define _SC_TZNAME_MAX       0x0000000d // # of bytes in timezone name
/* POSIX.2 values */
#define _SC_BC_BASE_MAX      0x0000000e
#define _SC_BC_DIM_MAX       0x0000000f
#define _SC_BC_SCALE_MAX     0x00000010
#define _SC_BC_STRING_MAX    0x00000011
#define _SC_COLL_WEIGHTS_MAX 0x00000012
#define _SC_EXPR_NEST_MAX    0x00000013
#define _SC_LINE_MAX         0x00000014
#define _SC_2_VERSION        0x00000015
#define _SC_2_C_DEV          0x00000016
#define _SC2_FORT_DEV        0x00000017
#define _SC_2_LOCALEDEF      0x00000018
#define _SC2_SW_DEV          0x00000019
/* non-standard values */
/* Unix/Linux extensions */
#define _SC_PHYS_PAGES       -1 // # of pages of physical memory
#define _SC_AVPHYS_PAGES     -2 // # of available physical pages
#define _SC_NPROCESSORS_CONF -3 // # of processors configured
#define _SC_NPROCESSORS_ONLN -4 // # of processors currently online
/* Zero extensions */
#define _SC_CACHELINESIZE    -5 // size of cacheline in bytes
#define _SC_L1INSTSIZE       -6 // L1 cache size in bytes
#define _SC_L1DATASIZE       -7 // L1 cache size in bytes
#define _SC_L1INSTNWAY       -8 // L1 cache parameter
#define _SC_L1DATANWAY       -9 // L1 cache parameter
#define _SC_L2SIZE           -10 // L2 cache size in bytes
#define _SC_L2NWAY           -11 // L2 cache parameter
#define _SC_BLKSIZE          -12 // buffer block size
#define MINSYSCONF           (-12)
#define NSYSCONF             (26 - MINSYSCONF) // 1 bigger than any name-value
#if (USEXOPENEXT) && !defined(F_LOCK)
/* these macros also appear in <fcntl.h> - keep the files consistent */
#define F_ULOCK 0       // unlock section of a file
#define F_LOCK  1       // lock section
#define F_TLOCK 2       // try lock
#define F_TEST  3       // test lock
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

