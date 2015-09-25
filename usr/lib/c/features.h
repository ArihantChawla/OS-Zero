#ifndef __FEATURES_H__
#define __FEATURES_H__

#define _REENTRANT             1        // enable re-entrant thread-safe code
#define _THREAD_SAFE           1        // thread safety on some systems

/* feature macro list */
#define _ISOC11_SOURCE         1        // ISO 9899-2011
#define _ISOC99_SOURCE         1        // ISO 9899-1999
#define _FILE_OFFSET_BITS      64       // enable 64-bit off_t
#define _LARGEFILE_SOURCE      1        // enable 64-bit file-related types
#define _LARGEFILE64_SOURCE    1        // enable 64-bit file-related types
#define _XOPEN_SOURCE          700      // XOPEN version
#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 700)                    \
    && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE        200809L
#elif defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 600)                  \
    && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE        200112L
#elif defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)                  \
    && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE        199506L
#endif
#if defined(_XOPEN_SOURCE) && (_XOPEN_SOURCE >= 500)
#define _XOPEN_SOURCE_EXTENDED 1        // activate X/Open extensions
#if defined(_POSIX_C_SOURCE)
#define _POSIX_SOURCE          1        // old macro for POSIX features
#endif
#if defined(_POSIX_C_SOURCE) && (_POSIX_C_SOURCE >= 200809L)
#define _ATFILE_SOURCE
#endif
#define _SVID_SOURCE           1
#define _GNU_SOURCE            1        // enable GNU extensions
#define _DEFAULT_SOURCE        1
#define _BSD_SOURCE            1        // enable BSD features
#undef  _FAVOR_BSD
#define _ZERO_SOURCE           1        // enable Zero features; version number
#define _UNIX_SOURCE           1        // enable Unix features
#define _MSDOS_SOURCE          1        // enable [some] MS-DOS style C features
#define _QNX_SOURCE            1        // [possible] QNX features

#define USEXOPEN               (defined(_XOPEN_SOURCE))
#define USEXOPEN2K             (defined(_XOPEN_SOURCE)                  \
                                && (_XOPEN_SOURCE >= 600))
#define USEXOPEN2K8            (defined(_XOPEN_SOURCE)                  \
                                && (_XOPEN_SOURCE >= 700))
#define USEXOPENEXT            (defined(_XOPEN_SOURCE_EXTENDED))
#define USEUNIX95              (defined(_XOPEN_SOURCE_EXTENDED))
#define USEUNIX98              (defined(_XOPEN_SOURCE)                  \
                                && (_XOPEN_SOURCE >= 500))
#define USEPOSIX               (defined(_POSIX_SOURCE)                  \
                                || defined(_XOPEN_SOURCE))
#define USEPOSIX2              ((defined(_POSIX_SOURCE)                 \
                                 && (_POSIX_SOURCE > 1))                \
                                || defined(_XOPEN_SOURCE))
#define USEPOSIX199309         (defined(_POSIX_C_SOURCE)                \
                                && (_POSIX_C_SOURCE >= 199309L))
/* thread extensions in POSIX.1-1995 */
#define USEPOSIX199506         (defined(_POSIX_C_SOURCE)                \
                                && (_POSIX_C_SOURCE >= 199506L))
#define USEPOSIX200112         (defined(_POSIX_SOURCE)                  \
                                && _POSIX_C_SOURCE >= 200112L)

#define USEBSD                 (defined(_BSD_SOURCE))
/* favor old school BSD interfaces */
#define FAVORBSD               (defined(_BSD_SOURCE) && defined(_FAVOR_BSD))
#define USESVID                (defined(_SVID_SOURCE))
#define USEGNU                 (defined(_GNU_SOURCE))
#define USEOLDBSD              FAVORBSD

/*
 * _POSIX_VERSION
 * --------------
 * 198808L for POSIX.1-1988
 * 199009L for POSIX.1-1990
 * 199309L for POSIX1.b (real-time extensions)
 * 199506L for POSIX.1c (threads)
 * 200112L for POSIX.1-2001
 * 200809L for POSIX.1-2008
 *
 * POSIX macros
 * ------------
 * _POSIX_IPV6 indicates IPv6 address support
 * _POSIX_MAPPED_FILES indicates memory mapping support
 * _POSIX_SEMAPHORES indicates semaphore support multi-threading
 * _POSIX_THREADS indicates pthread support
 */

/* NOTE: see posixopts(7) for fixing the mess below... =) */
/*
 * true indicates the following functions are present
 * - posix_fadvise()
 * - posix_fallocate()
 * - posix_memalign()
 * - posix_madvise()
 */
#define _POSIX_ADVISORY_INFO   0
/*
 * the header <aio.h> is present together with the following functions
 * - aio_cancel()
 * - aio_error()
 * - aio_fsync()
 * - aio_read()
 * - aio_return()
 * - aio_suspend()
 * - aio_write()
 * - lio_listio()
 */
#define _POSIX_ASYNCHRONOUS_IO 0

#endif /* __FEATURES_H__ */

