#ifndef __FEATURES_H__
#define __FEATURES_H__

#define _REENTRANT             1
#define _THREAD_SAFE           1        // thread safety
#define USEXOPEN2K             1
#define USEXOPENEXT            1
#define USEUNIX98              1
#define USEPOSIX               1
#define USEPOSIX2              1
#define USEPOSIX199309         1
#define USEPOSIX199506         1
#define USEBSD                 1
#define USEOLDBSD              0
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
#define _ZERO_SOURCE           1        // enable zero features
#define _UNIX_SOURCE           1        // enable Unix features

#endif /* __FEATURES_H__ */

