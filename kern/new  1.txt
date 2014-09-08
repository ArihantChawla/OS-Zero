#ifndef __KERN_VERSION_H__
#define __KERN_VERSION_H__

#define _ZERO_VERSION                     (_ZERO_MAKE_VERSION(ZERO_MAJOR_VERSION,
                                                              ZERO_MINOR_VERSION,
															  ZERO_PATCH_LEVEL))
#define _ZERO_MAJOR_VERSION               0U
#define _ZERO_MINOR_VERSION               0U
#define _ZERO_PATCH_LEVEL                 0U
#define _ZERO_MAKE_VERSION(maj, min, pat) (((maj) << 22) | ((min) << 11) | (pat))

#endif /* __KERN_VERSION_H__ */

