#include <stddef.h>
#include <time.h>
#include <errno.h>
#if defined(PTHREAD)
#include <pthread.h>
#endif
#include <zero/mtx.h>
#include <zero/cond.h>
#include <zero/thr.h>

void
condinit(zerocond *cond)
{
    mtxinit(&cond->queue.lk);
    cond->queue.head = NULL;
    cond->queue.tail = NULL;
}

long
condwait(zerocond *cond, zeromtx *mtx)
{
    if (!cond || !mtx) {

        return EINVAL;
    } else if (!mtxtrylk(mtx)) {

        return EPERM;
    } else {
        thrwait1(&cond->queue);
    }
    zerolkmtx(mtx);

    return 0;
}

long
condwaitabstime(zerocond *cond, zeromtx *mtx, const struct timespec *absts)
{
    if (!cond || !mtx || !absts || absts->tv_sec < 0
        || absts->tv_nsec < 0 || absts->tv_nsec >= 1000000000) {

        return EINVAL;
    } else if (!mtxtrylk(mtx)) {

        return EPERM;
    } else if (thrsleep2(&cond->queue, absts) < 0) {

        return EINVAL;
    }
    zerolkmtx(mtx);

    return 0;
}

long
condsignal(zerocond *cond)
{
    if (!cond) {

        return EINVAL;
    }
    thrwake1(&cond->queue);

    return 0;
}

long
condsignal2(zerocond *cond, long nthr)
{
    if (!cond || !nthr) {

        return EINVAL;
    }
    while (nthr--) {
        thrwake1(&cond->queue);
    }

    return 0;
}

long
condbroadcast(zerocond *cond)
{
    if (!cond) {

        return EINVAL;
    }
    thrwakeall1(&cond->queue);

    return 0;
}

