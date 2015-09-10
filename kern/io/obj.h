#ifndef __KERN_IO_OBJ_H__
#define __KERN_IO_OBJ_H__

struct iocdev {
    long  major;
    long  minor;
    char *path;
    struct iofuncs *funcs;
};

struct iobdev {
};

#endif /* __KERN_IO_OBJ_H__ */

