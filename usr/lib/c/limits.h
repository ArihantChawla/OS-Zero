#ifndef __LIMITS_H__
#define __LIMITS_H__

#include <features.h>
#include <stdint.h>
#include <zero/param.h>
#if (_ZERO_SOURCE)
#include <kern/conf.h>
#include <kern/signal.h>
#endif

#if 0 /* unnecessary with C99 */
/* integral-value limits */
#define WORD8_MIN     (-0x7f - 1)
#define WORD8_MAX     0x7f
#define UWORD8_MAX    0xff
#define WORD16_MIN    (-0x7fff - 1)
#define WORD16_MAX    0x7fff
#define UWORD16_MAX   0xffff
#define WORD32_MIN    (-0x7fffffff - 1)
#define WORD32_MAX    0x7fffffff
#define UWORD32_MAX   0xffffffff
#define WORD64_MIN    INT64_C(-0x7fffffffffffffff - 1)
#define WORD64_MAX    INT64_C(0x7fffffffffffffff)
#define UWORD64_MAX   UINT64_C(0xffffffffffffffff)
#endif

/*
 * ISO limits.
 */
/*
 * <FIXME>
 * - these should be detected by configure
 * </FIXME>
 */
#define CHAR_BIT      8
#define CHAR_MAX      WORD8_MAX
#define CHAR_MIN      WORD8_MIN
#define SCHAR_MAX     WORD8_MAX
#define SCHAR_MIN     WORD8_MIN
#define UCHAR_MAX     UWORD8_MAX
#define SHRT_MIN      WORD16_MIN
#define SHRT_MAX      WORD16_MAX
#define USHRT_MAX     UWORD16_MAX
#define INT_MAX       WORD32_MAX
#define INT_MIN       WORD32_MIN
#define UINT_MAX      UWORD32_MAX
#if (LONGSIZE == 4)
#   define LONG_MAX   WORD32_MAX
#   define LONG_MIN   WORD32_MIN
#   define ULONG_MAX  UWORD32_MAX
#elif (LONGSIZE == 8)
#   define LONG_MAX   WORD64_MAX
#   define LONG_MIN   WORD64_MIN
#   define ULONG_MAX  UWORD64_MAX
#endif
#   define LLONG_MIN  WORD64_MIN
#   define LLONG_MAX  WORD64_MAX
#   define ULLONG_MAX UWORD64_MAX

#define MB_LEN_MAX    1

#if defined(_POSIX_SOURCE) && (_POSIX_C_SOURCE >= 200112L)

/*
 * POSIX limits.
 */
/*
 * Invariant minimum values.
 */
#define _POSIX_ARG_MAX     4096
#define _POSIX_CHILD_MAX   6
#define _POSIX_LINK_MAX    8
#define _POSIX_MAX_CANON   255
#define _POSIX_MAX_INPUT   255
#define _POSIX_NAME_MAX    14
#define _POSIX_NGROUPS_MAX 16		// number of supplementary group IDs if available
#define _POSIX_OPEN_MAX    16
#define _POSIX_PATH_MAX    255
#define _POSIX_PIPE_BUF    512
#define _POSIX_SSIZE_MAX   32767
#define _POSIX_STREAM_MAX  8
#define _POSIX_TZNAME_MAX  3
/*
 * Default values for possibly indeterminate run-time invariant values
 */
/*
 * POSIX values.
 */
#define ARG_MAX     65536 	// # of arg and env bytes to exec functions
#define CHILD_MAX   256   	// # of processes per real user ID
#define LINK_MAX    127         // # of links per file
#define MAX_CANON   255   	// size of canonical input queue
#define MAX_INPUT   255         // # size of type-ahead buffer
#define NGROUPS_MAX 16    	// # of supplementary group ID per process
#define OPEN_MAX    32768 	// # of open files per process
#define PIPE_BUF    4096        // # of bytes in atomic write to pipe
#define STREAM_MAX  FOPEN_MAX   // # of open I/O streams per process
#define NAME_MAX    255         // # of bytes in file names
#define PATH_MAX    4096        // # of bits in path including terminating NUL
#define TZNAME_MAX  255         // # of bytes in timezone names

#endif /* POSIX */
#define RTSIG_MAX   NRTSIG

/*
 * Unix values
 */
#define PASS_MAX    8 /* maximum significant characters in password */
#define LOGIN_MAX   8 // maximum significant characters in login name */

/*
 * Determinate (compile-time) values.
 */
#define SSIZE_MAX   LONG_MAX

#if (_POSIX_SOURCE)
#if (_ZERO_SOURCE)
#define _POSIX_FD_SETSIZE NPROCFD
#else
#define _POSIX_FD_SETSIZE 32768
#endif
#endif /* _POSIX_SOURCE */

#endif /* __LIMITS_H__ */

