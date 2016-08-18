#ifndef __ZERO_MACROS_H__
#define __ZERO_MACROS_H__

#if defined(__GNUC__)
#define containerof(ptr, type, member)                                  \
    ({                                                                  \
        const typeof(((type *)0)->member) *_mptr = (ptr);               \
        (type *)((char *)_mptr - offsetof(type, member));               \
    })
#endif /* __GCC__ */

#endif /* __ZERO_MACROS_H__ */

