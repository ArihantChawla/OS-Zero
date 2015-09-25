#ifndef __FEATURES_H__
#define __FEATURES_H__

#define _REENTRANT             1        // enable re-entrant thread-safe code
#define _THREAD_SAFE           1        // thread safety on some systems

#define USEXOPEN               (defined(_XOPEN_SOURCE))
#define USEXOPEN2K             1
#define USEXOPEN2K8            1
#define USEXOPENEXT            (defined(_XOPEN_SOURCE_EXTENDED))
#define USEUNIX98              1
#define USEPOSIX               (defined(_POSIX_SOURCE))
#define USEPOSIX2              (defined(_POSIX_SOURCE) && (_POSIX_SOURCE > 1))
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

/* feature macro list */
#define _ISOC11_SOURCE         1        // ISO 9899-2011
#define _ISOC99_SOURCE         1        // ISO 9899-1999
#define _FILE_OFFSET_BITS      64       // enable 64-bit off_t
#define _LARGEFILE_SOURCE      1        // enable 64-bit file-related types
#define _LARGEFILE64_SOURCE    1        // enable 64-bit file-related types
#define _POSIX_SOURCE          1        // enable POSIX features
#define _POSIX_C_SOURCE        200112L	// POSIX version
#define _XOPEN_SOURCE          600      // XOPEN version
#define _XOPEN_SOURCE_EXTENDED 1        // activate X/Open extensions
#define _SVID_SOURCE           1
#define _GNU_SOURCE            1        // enable GNU extensions
#define _BSD_SOURCE            1        // enable BSD features
#undef  _FAVOR_BSD
#define _ZERO_SOURCE           1        // enable Zero features; version number
#define _UNIX_SOURCE           1        // enable Unix features
#define _MSDOS_SOURCE          1        // enable [some] MS-DOS style C features
#define _QNX_SOURCE            1        // [possible] QNX features

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

