#include <errno.h>
#include <pthread.h>
#include <zero/asm.h>
#define ZEROMTX 1
#define ZEROPTHREAD 1
#include <zero/mtx.h>
#include <zero/sem.h>
//#include <zero/thr.h>

long
semwait(zerosem *sem)
{
    do {
#if defined(PTHREAD)
        while (!pthread_mutex_trylock(&sem->lk)) {
            thryield();
        }
#elif defined(ZEROMTX)
        while (m_cmpswap(&sem->lk, MTXINITVAL, MTXLKVAL)) {
            thryield();
        }
#endif
        if (sem->val > 0) {
            sem->val--;
#if defined(PTHREAD)
            pthread_mutex_unlock(&sem->lk);
#elif defined(ZEROMTX)
            sem->lk = MTXINITVAL;
#endif

            return 0;
        } else {
#if defined(PTHREAD)
            pthread_mutex_unlock(&sem->lk);
#elif defined(ZEROMTX)
            sem->lk = MTXINITVAL;
#endif
            thryield();
        }
    } while (1);

    return 0;
}

long
semtrywait(zerosem *sem)
{
    do {
#if defined(PTHREAD)
        while (!pthread_mutex_trylock(&sem->lk)) {
            thryield();
        }
#elif defined(ZEROMTX)
        while (m_cmpswap(&sem->lk, MTXINITVAL, MTXLKVAL)) {
            thryield();
        }
#endif
        if (sem->val > 0) {
            sem->val--;
#if defined(PTHREAD)
            pthread_mutex_unlock(&sem->lk);
#elif defined(ZEROMTX)
            sem->lk = MTXINITVAL;
#endif

            return 0;
        } else {
#if defined(PTHREAD)
            pthread_mutex_unlock(&sem->lk);
#elif defined(ZEROMTX)
            sem->lk = MTXINITVAL;
#endif
            errno = EAGAIN;

            return -1;
        }
    } while (1);

    return 0;
}

long
sempost(zerosem *sem)
{
    do {
#if defined(PTHREAD)
        while (!pthread_mutex_trylock(&sem->lk)) {
            thryield();
        }
#elif defined(ZEROMTX)
        while (m_cmpswap(&sem->lk, MTXINITVAL, MTXLKVAL)) {
            thryield();
        }
#endif
        if (!sem->val) {
#if defined(PTHREAD)
            pthread_mutex_unlock(&sem->lk);
#elif defined(ZEROMTX)
            sem->lk = MTXINITVAL;
#endif
            thryield();
        } else if (sem->val != ZEROSEM_MAXVAL) {
            sem->val++;
#if defined(PTHREAD)
            pthread_mutex_unlock(&sem->lk);
#elif defined(ZEROMTX)
            sem->lk = MTXINITVAL;
#endif

            return 0;
        } else {
#if defined(EOVERFLOW)
            errno = EOVERFLOW;
#endif
            
            return -1;
        }
    } while (1);

    return 0;
}

