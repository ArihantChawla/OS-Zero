#ifndef __SYS_ZERO_CONF_H__
#define __SYS_ZERO_CONF_H__

#if defined(__KERNEL__)

#include <kern/conf.h>

#define ZEROKBDUS      KBDUS

#else

#define ZEROKBDUS      1

#endif

#define ZEROPS2KBDSET2 1

#endif /* __SYS_ZERO_CONF_H__ */

