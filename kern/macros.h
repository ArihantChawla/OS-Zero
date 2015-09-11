#ifndef __KERNEL_MACROS_H__
#define __KERNEL_MACROS_H__

/* this beast comes from the Linux kernel :) */
#define container(ptr, type, member)                                    \
    ({                                                                  \
        const typeof(((type *)0)->member) *_mptr;                       \
        (type *)((char *)_mptr - offsetof(type, member));               \
    })
        

#endif /* __KERNEL_MACROS_H__ */

