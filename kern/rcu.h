#ifndef __KERN_RCU_H__
#define __KERN_RCU_H__

#include <kern/list.h>

struct rcuhead {
    struct   listhead list;
    void   (*func)(void *obj);
    void    *arg;
};

#endif /* __KERN_RCU_H__ */

