#include <time.h>
#include <sys/time.h>
#include <zero/cdecl.h>
#include <zero/asm.h>
#include <zero/thr.h>
#include <zero/cond.h>
#include <zero/prof.h>
#define LIST_TYPE         zerothr
#define LIST_QTYPE        zerothrqueue
#define LIST_RM_COND(thr) ((thr)->sleep)
#define LIST_QUEUE(thr)   ((thr)->sleep = ZEROTHR_ASLEEP)
#define LIST_DEQUEUE(thr) ((thr)->sleep = ZEROTHR_AWAKE)
#include <zero/list.h>

static zerothrqueue thrsleepqueue;
THREADLOCAL zerothr thrself;

void
thrwait1(zerothrqueue *queue)
{
    zerothr *thr = &thrself;

    if (!queue) {
        queue = &thrsleepqueue;
    }
    listpush(queue, thr);
    while (m_atomread(thr->sleep)) {
        thryield();
    }
             
    return;
}

long
thrsleep2(zerothrqueue *queue, const struct timespec *absts)
{
    zerothr        *thr = &thrself;
#if defined(USECLOCKNANOSLEEP)
    struct timespec tsrem;
#else
    struct timeval  tvout;
    struct timeval  tvcur;
#endif

    if (!queue) {
        queue = &thrsleepqueue;
    }
    listpush(queue, thr);
#if defined(USECLOCKNANOSLEEP)
    while (clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, absts, &tsrem)) {
        if (errno == EINTR) {
            
            continue;
        } else {
            if (m_atomread(thr->sleep)) {
                listrm(queue, thr);
            }
            
            return -1;
        }
    }
    if (m_atomread(thr->sleep)) {
        listrm(queue, thr);
    }
#else
    gettimeofday(&tvcur, NULL);
    TIMESPEC_TO_TIMEVAL(&tvout, absts);
    do {
        if (tvcmp(&tvout, &tvcur) > 0) {
            if (m_atomread(thr->sleep)) {
                listrm(queue, thr);
            }
            
            return 0;
        }
        thryield();
        gettimeofday(&tvcur, NULL);
    } while (1);
#endif

    return 0;
}

void
thrwake1(zerothrqueue *queue)
{
    zerothr *thr = NULL;

    if (!queue) {
        queue = &thrsleepqueue;
    }
    listdequeue(queue, &thr);

    return;
}

void
thrwakeall1(zerothrqueue *queue)
{
    zerothr *thr = NULL;

    if (!queue) {
        queue = &thrsleepqueue;
    }
    do {
        listdequeue(queue, &thr);
    } while (thr);

    return;
}

