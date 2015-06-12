#ifndef __DIRENT_H__
#define __DIRENT_H__

#define PATH_MAX 4095
#define NAMEMAX  255

#include <stdint.h>
#include <zero/cdecl.h>

#if (__GLIBC__)
typedef struct __dirstream DIR;
#endif

struct dirent *readdir(DIR *dirp);

struct dirent {
    ino_t    d_ino;
    off_t    d_off;
    uint16_t d_reclen;
    uint8_t  d_type;
    uint8_t  d_name[NAMEMAX + 1];
} PACK();

#endif /* __DIRENT_H__ */

