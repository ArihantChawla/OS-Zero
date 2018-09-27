#ifndef __KERN_IO_OBJ_H__
#define __KERN_IO_OBJ_H__

#include <stdint.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <zero/spinrw.h>
#include <kern/macros.h>
#include <kern/obj.h>
#include <kern/list.h>
#include <kern/sparse.h>
#include <kern/io/vfs.h>

struct fileops {
    void     *drv:
    long    (*open)(struct desc *, struct file *);
    long    (*lock)(struct desc *, long op, struct filelock *); /* FIXME */
    loff_t  (*seek)(struct desc *, loff_t, long);
    ssize_t (*read)(struct desc *, __user void *, size_t, loff_t *);
    ssize_t (*write)(struct desc *, const void __user *, size_t, loff_t *);
    ssize_t (*readv)(struct desc *, const struct iovec *, size_t, loff_t *);
    ssize_t (*writev)(struct desc *, const struct iovec *, size_t, loff_t *);
    long    (*close)(struct desc *, struct file *);
};

struct fileatr {
    struct spinrw  lk;
    long           pidspec;     // processes SIGIO is sent to
    struct        *pid;         // pid or -pgrp target for SIGIO
    uid_t          uid;
    uid_t          euid;
    int            sigrt;       // POSIX 1.b signal delivered on IO
};

struct file {
    union {
        struct listhead  list;          // list of file objects
        struct rcuhead   rcu;           // Linux-style RCU list after freeing
    } objlist;
    struct vfspath       vfspath;       // VFS location
    struct fileops      *ops;           // file operation structure
    volatile long        spin;          // lock
    volatile long        nref;          // reference count
    long                 flg;           // flags specified for open()
    mode_t               mode;
    loff_t               ofs;
};

#endif /* __KERN_IO_OBJ_H__ */

