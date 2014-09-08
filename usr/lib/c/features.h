#ifndef __FEATURES_H__
#define __FEATURES_H__

#define _REENTRANT             1        // enable re-entrant thread-safe code
#define _THREAD_SAFE           1        // thread safety
#define USEXOPEN2K             1
#define USEXOPENEXT            1
#define USEUNIX98              1
#define USEPOSIX               1
#define USEPOSIX2              1
#define USEPOSIX199309         1
#define USEPOSIX199506         1
#define USEBSD                 1
#define USEOLDBSD              0        // favor old school BSD interfaces
#define USESVID                1
#define USEGNU                 1

/* feature macro list */
#define _ISOC11_SOURCE         1        // ISO 9899-2011
#define _ISOC99_SOURCE         1        // ISO 9899-1999
#define _FILE_OFFSET_BITS      64       // enable 64-bit off_t
#define _LARGEFILE_SOURCE      1 
#define _LARGEFILE64_SOURCE    1
#define _POSIX_SOURCE          1        // enable POSIX
#define _POSIX_C_SOURCE        200112L	// POSIX version
#define _XOPEN_SOURCE          600      // XOPEN version
#define _XOPEN_SOURCE_EXTENDED 1
#define _GNU_SOURCE            1        // enable GNU extensions
#define _BSD_SOURCE            1        // enable BSD features
#define _ZERO_SOURCE           1        // enable Zero features
#define _UNIX_SOURCE           1        // enable Unix features

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

#endif /* __FEATURES_H__ */

