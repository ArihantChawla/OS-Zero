#ifndef __KERN_TIME_H__
#define __KERN_TIME_H__

#include <stdint.h>
#include <sys/types.h>

#define TIME_SECOND 1000000000    // nanosecond resolution

time_t kcurtime(void);

#endif /* __KERN_TIME_H__ */

