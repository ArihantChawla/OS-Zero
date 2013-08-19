#ifndef __SYS_TIME_H__
#define __SYS_TIME_H__

#include <features.h>
#include <sys/select.h>

#ifndef CLK_TCK
#define CLK_TCK        CLOCKS_PER_SEC
#endif

/* FIXME: not sure if this the place for time_t... */
typedef uint64_t time_t;

#endif /* __SYS_TIME_H__ */

