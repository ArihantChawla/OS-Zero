#ifndef __KERN_VERSION_H__
#define __KERN_VERSION_H__

#define _ZERO_MAKE_VERSION(maj, min, pat) (((maj) << 22)                \
                                           | ((min) << 11) | (pat))
#define _ZERO_MAJOR_VERSION               0U
#define _ZERO_MINOR_VERSION               0U
#define _ZERO_PATCH_LEVEL                 0U
#define _ZERO_VERSION                                                   \
    (_ZERO_MAKE_VERSION(_ZERO_MAJOR_VERSION,                            \
                        _ZERO_MINOR_VERSION,                            \
                        _ZERO_PATCH_LEVEL))

#endif /* __KERN_VERSION_H__ */

