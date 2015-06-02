#ifndef __KERN_IO_H__
#define __KERN_IO_H__

#include <stdint.h>
#include <sys/types.h>

#if 0
#define SEEK_CUR        0x00
#define SEEK_SET        0x01
#define SEEK_END        0x02
#endif

struct file {
    off_t      ofs;
    uintptr_t  desc;
    char      *path;
    long       flg;
    long       errflg;
    
};

/* TODO: implement iogetc() */
#define iogetc(fp)

#endif /* __KERN_IO_H__ */

