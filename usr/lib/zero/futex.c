/* TODO: implement linux-like futexes (fast userspace mutexes) */
#include <features.h>
#if defined(__linux__)
#if !defined(_GNU_SOURCE)
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>
#endif
#endif
#include <zero/asm.h>
#include <zero/mtx.h>
#include <zero/futex.h>

#if defined(__linux__)
long syscall(long num, ...);
#endif

long
mutex_init(mutex_t *mutex, const pthread_mutexattr_t *atr)
{
    (void)atr;
    *mutex = ZEROMTXINITVAL;

    return 0;
}

long
mutex_destroy(mutex_t *mutex)
{
    (void)mutex;
    
    return 0;
}

mutex_t
mutex_lock(mutex_t *mutex)
{
    volatile long mtx = 0L;
    long          l;
    
    mtx = m_cmpswap(mutex, ZEROMTXINITVAL, ZEROMTXLKVAL);
    if (mtx != ZEROMTXLKVAL) {
        if (mtx == ZEROMTXCONTVAL) {
            mtx = m_cmpswap(mutex, ZEROMTXCONTVAL, ZEROMTXLKVAL);
            
            return mtx;
        } else {
            for (l = 0 ; l < 100; l++) {
                mtx = m_cmpswap(mutex, ZEROMTXINITVAL, ZEROMTXLKVAL);
                if (!mtx) {
                    mtx = m_cmpswap(mutex, ZEROMTXCONTVAL, ZEROMTXLKVAL);
                    if (!mtx) {
                        m_waitint();
                    }
                }
            }
        }
    }
        
    return mtx;
}

mutex_t
mutex_unlock(mutex_t *mutex)
{
    volatile long mtx = m_cmpswap(mutex, ZEROMTXLKVAL, ZEROMTXINITVAL);

    if (mtx) {
        mtx = m_cmpswap(mutex, ZEROMTXCONTVAL, ZEROMTXINITVAL);
    }

    return mtx;
}

