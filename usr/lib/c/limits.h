#ifndef __LIMITS_H__
#define __LIMITS_H__

#include <zero/param.h>

#define WORD8_MIN    (-0x7f - 1)
#define WORD8_MAX    0x7f
#define UWORD8_MAX   0xff
#define WORD16_MIN   (-0x7fff - 1)
#define WORD16_MAX   0x7fff
#define UWORD16_MAX  0xffff
#define WORD32_MIN   (-0x7fffffff - 1)
#define WORD32_MAX   0x7fffffff
#define UWORD32_MAX  0xffffffff
#define WORD64_MIN   __INT64_C(-0x7fffffffffffffff - 1)
#define WORD64_MAX   __INT64_C(0x7fffffffffffffff)
#define UWORD64_MAX  __UINT64_C(0xffffffffffffffff)
#if 0
#define WORD128_MIN  (-0x7fffffffffffffff - 1)
#define WORD128_MAX  0x7fffffffffffffff
#define UWORD128_MAX 0xffffffffffffffffffffffffffffffff
#define WORD256_MIN \
    (-0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff - 1)
#define WORD256_MAX \
    (0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
#define UWORD256_MAX \
    0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff
#endif

#define LOGIN_MAX 8 /* maximum significant characters in login name. */

/*
 * ISO limits.
 */
/*
 * <FIXME>
 * - these should be detected by configure.
 * </FIXME>
 */
#define CHAR_BIT     8
#define CHAR_MAX     WORD8_MAX
#define CHAR_MIN     WORD8_MIN
#define SCHAR_MAX    WORD8_MAX
#define SCHAR_MIN    WORD8_MIN
#define UCHAR_MAX    UWORD8_MAX
#define SHRT_MIN     WORD16_MIN
#define SHRT_MAX     WORD16_MAX
#define USHRT_MAX    UWORD16_MAX
#define INT_MAX      WORD32_MAX
#define INT_MIN      WORD32_MIN
#define UINT_MAX     UWORD32_MAX
#if (LONGSIZE == 4)
#   define LONG_MAX   WORD32_MAX
#   define LONG_MIN   WORD32_MIN
#   define ULONG_MAX  UWORD32_MAX
#   define LLONG_MIN  WORD64_MIN
#   define LLONG_MAX  WORD64_MAX
#   define ULLONG_MAX UWORD64_MAX
#elif (LONGSIZE == 8)
#   define LONG_MAX   WORD64_MAX
#   define LONG_MIN   WORD64_MIN
#   define ULONG_MAX  UWORD64_MAX
#elif (LONGSIZE == 16)
#   define LONG_MAX   WORD128_MAX
#   define LONG_MIN   WORD128_MIN
#   define ULONG_MAX  UWORD128_MAX
#elif (LONGSIZE == 32)
#   define LONG_MAX   WORD256_MAX
#   define LONG_MIN   WORD256_MIN
#   define ULONG_MAX  UWORD256_MAX
#endif
#define MB_LEN_MAX   1

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
#define _POSIX_NGROUPS_MAX 0
#define _POSIX_OPEN_MAX    16
#define _POSIX_PATH_MAX    255
#define _POSIX_PIPE_BUF    512
#define _POSIX_SSIZE_MAX   32767
#define _POSIX_STREAM_MAX  8
#define _POSIX_TZNAME_MAX  3
/*
 * Default values for possibly indeterminate run-time invariant values.
 */
/*
 * POSIX values.
 */
#define ARG_MAX     65536 /* argument bytes to exec functions */
#define CHILD_MAX   256   /* number of processes per real user ID */
#define NGROUPS_MAX 16    /* number of supplementary group ID per process */
#define OPEN_MAX    256   /* maximum number of open files per process */
#define STREAM_MAX  FOPEN_MAX
#define NAME_MAX    255 
#define PATH_MAX    (PAGE_SIZE - 1)
#define TZNAME_MAX  255
/*
 * Unix values.
 */
#define PASS_MAX    8 /* maximum significant characters in password */
/*
 * Determinate (compile-time) values.
 */
#if (LONGSIZE == 4)
#   define SSIZE_MAX WORD32_MAX
#elif (LONGSIZE == 8)
#   define SSIZE_MAX WORD64_MAX
#elif (LONGSIZE == 16)
#   define SSIZE_MAX WORD128_MAX
#elif (LONGSIZE == 32)
#   define SSIZE_MAX WORD256_MAX
#endif

#endif /* __LIMITS_H__ */

