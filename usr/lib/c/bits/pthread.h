#ifndef __BITS_PTHREAD_H__
#define __BITS_PTHREAD_H__

#include <features.h>
#undef ZEROTHR
#undef ZEROMTX
#define ZEROTHR 1
#define ZEROMTX 1
#include <mach/param.h>
#include <mt/thr.h>
#include <mt/spin.h>
#include <mt/mtx.h>
#include <sched.h>

#define PTHREAD_STACK_MIN        16384

typedef uintptr_t     pthread_t;
typedef struct thratr pthread_attr_t;

struct __pthread {
    zerothratr        atr;
    struct __pthread *prev;
    struct __pthread *next;
};

#define PTHREAD_KEY_SIZE PTRSIZE
typedef uintptr_t pthread_key_t;

struct __pthread_key {
    pthread_key_t          key;
    void                 (*destroy)(void *);
    void                  *val;
    unsigned long          hash;
    struct __pthread_key  *prev;
    struct __pthread_key  *next;
};

typedef zerospin pthread_spinlock_t;

/* prioceil values */
#define __PTHREAD_PRIOCEIL_MIN
/* mutex types */
#define PTHREAD_MUTEX_NORMAL     0
#define PTHREAD_MUTEX_RECURSIVE  1
#define PTHREAD_MUTEX_ERRORCHECK 2
#define PTHREAD_MUTEX_DEFAULT    PTHREAD_MUTEX_NORMAL
/* mutex protocols */
#define PTHREAD_PRIO_NONE        0
#define PTHREAD_PRIO_INHERIT     1
#define PTHREAD_PRIO_PROTECT     2
/* pshared values */
#define PTHREAD_PROCESS_PRIVATE  0
#define PTHREAD_PROCESS_SHARED   1
/* robust values */
#define PTHREAD_MUTEX_STALLED    0
#define PTHREAD_MUTEX_ROBUST     1
/* initialiser for default values */
#define PTHREAD_MUTEX_INITIALIZER                                       \
    { MTXINITVAL,                                                       \
      0,                                                                \
      PTHREAD_MUTEX_NORMAL,                                             \
      PTHREAD_PRIO_NONE,                                                \
      PTHREAD_PROCESS_PRIVATE,                                          \
      PTHREAD_MUTEX_STALLED }
typedef struct {
//    volatile long lk;
    zeromtx  mtx;
    unsigned prioceil : 8;
    unsigned type     : 8;
    unsigned proto    : 2;
    unsigned pshared  : 1;
    unsigned robust   : 1;
} pthread_mutex_t;

typedef zeromtxatr pthread_mutexattr_t;

#endif /* __BITS_PTHREAD_H__ */

