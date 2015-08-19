#ifndef __KERN_IO_H__
#define __KERN_IO_H__

#include <stdint.h>
#include <sys/types.h>

struct file {
    long       lkcnt;
    pid_t      owner;
    off_t      ofs;
    uintptr_t  desc;
    char      *path;
    long       flg;
    long       err;
};

/* TODO: implement iogetc() */
#define iogetc(fp)

#endif /* __KERN_IO_H__ */

