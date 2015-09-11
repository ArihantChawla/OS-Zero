#ifndef __KERN_IO_OBJ_H__
#define __KERN_IO_OBJ_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <kern/obj.h>
#include <kern/list.h>
#include <kern/sparse.h>

struct iocdev {
    long               major;
    long               minor;
    char              *path;
    struct iodevfuncs *funcs;
};

struct iobdev {
    long               major;
    long               minor;
    char              *path;
    struct iodevfuncs *funcs;
};

struct file {
    struct listhead objlist;
};

struct iodevops {
    struct mod  *mod;
    long       (*open)(struct desc *, struct file *);
    long       (*lock)(struct desc *, ...); /* FIXME */
    loff_t     (*seek)(struct desc *, loff_t, long);
    ssize_t    (*read)(struct desc *, void *, size_t, loff_t *);
    ssize_t    (*write)(struct desc *, const void *, size_t, loff_t *);
    ssize_t    (*readv)(struct desc *, const struct iovec *, size_t, loff_t *);
    ssize_t    (*writev)(struct desc *, const struct iovec *, size_t, loff_t *);
    long       (*close)(struct desc *, struct file *);
};

#endif /* __KERN_IO_OBJ_H__ */

