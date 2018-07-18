#ifndef __MT_CONF_H_
#define __MT_CONF_H_

#if !defined(PTHREAD)
#define PTHREAD 0
#endif

#if (PTHREAD)
#include <pthread.h>
#define ZERO_MUTEX_TYPE pthread_mutex_t
#else
#include <mt/mtx.h>
#define ZERO_MUTEX_TYPE zerofmtx
#endif

#endif /* __MT_CONF_H_ */

