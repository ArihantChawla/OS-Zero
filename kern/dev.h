#ifndef __KERN_DEV_H__
#define __KERN_DEV_H__

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#include <unistd.h>

struct dev {
    long (*seek)(long, off_t, long);
};

#endif /* __KERN_DEV_H__ */

