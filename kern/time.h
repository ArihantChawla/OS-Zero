#ifndef __KERN_TIME_H__
#define __KERN_TIME_H__

#include <stdint.h>

#define KTIME_SECOND 1000000    // microsecond resolution

typedef int64_t ktime_t;

ktime_t kcurtime(void);

#endif /* __KERN_TIME_H__ */

