#ifndef __MT_CONF_H_
#define __MT_CONF_H_

#define ZERO_THREADS 0
#define ZERO_MUTEX   1
#if !defined(PTHREAD)
#define PTHREAD 0
#endif

#if (ZERO_MUTEX)
#include <mt/mtx.h>
#define ZERO_MUTEX_TYPE zerofmtx
#elif (PTHREAD)
#include <pthread.h>
#define ZERO_MUTEX_TYPE pthread_mutex_t
#endif

#endif /* __MT_CONF_H_ */

