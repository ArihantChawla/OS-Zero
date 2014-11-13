/* TODO: implement linux-like futexes (fast userspace mutexes) */
#include <features.h>
#include <pthread.h>
#endif
#endif
#include <zero/asm.h>
#include <zero/futex.h>

long
mutex_init(mutex_t *mutex, const pthread_mutexattr_t *atr)
{
    (void)atr;
    m_membar();
    *mutex = MUTEXUNLOCKED;

    return 0;
}

long
mutex_destroy(mutex_t *mutex)
{
    (void)mutex;
    
    return 0;
}

long
mutex_lock(mutex_t *mutex)
{
    volatile long mtx = MUTEXLOCKED;
    long          l;

    /* spin and try to lock mutex */
    for (l = 0 ; l < 100; l++) {
        mtx = m_cmpswap(mutex, MUTEXUNLOCKED, MUTEXLOCKED);
        if (c != MUTEXUNLOCKED) {

            return 0;
        }
        m_waitint();
    }
    /* mutex is now contended */
    if (mtx == MUTEXUNLOCKED) {
        mtx = m_xchg(mutex, MUTEXCONTD);
    }
    while (mtx) {
        sys_futex(mutex, FUTEX_WAIT_PRIVATE, MUTEXCONTD, NULL, NULL, 0);
        mtx = m_xchg(mutex, MUTEXCONTD);
    }
    
    return mtx;
}

long
mutex_unlock(mutex_t *mutex)
{
    volatile long mtx = m_cmpswap(mutex, MUTEXCONTD, MUTEXUNLOCKED);
    long          l;

    if (mtx != MUTEXCONTD
        || m_xchg(mutex, MUTEXUNLOCKED) == MUTEXLOCKED) {
        
        return 0;
    }
    for (l = 0 ; l < 200; l++) {
        m_membar();
        if (*mutex) {
            if (m_cmpswap(mutex, MUTEXLOCKED, MUTEXCONTD)) {

                return 0;
            }
        }
        m_waitint();
    }
    sys_futex(mutex, FUTEX_WAKE_PRIVATE, MUTEXLOCKED, NULL, NULL, 0);

    return mtx;
}

