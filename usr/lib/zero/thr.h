#ifndef __ZERO_THR_H__
#define __ZERO_THR_H__

#if defined(PTHREAD)
#include <pthread.h>
#endif

#if defined(PTHREAD)
#define thrid() ((long)pthread_self())
#endif

#if defined(_WIN64) || defined(_WIN32)
#define thryield() kYieldProcessor()
#elif defined(__linux__) && !defined(__KERNEL__)
#define thryield() sched_yield();
#elif defined(__KERNEL__)
#define thryield() schedyield();
#elif defined(PTHREAD)
#define thryield() pthread_yield();
#endif

#endif /* __ZERO_THR_H__ */

