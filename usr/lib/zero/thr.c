#include <zero/cdecl.h>
#include <zero/asm.h>
#include <zero/thr.h>
#include <zero/cond.h>
#define LIST_TYPE         zerothr
#define LIST_QTYPE        zerothrqueue
#define LIST_QUEUE(thr)   ((thr)->sleep = ZEROTHR_ASLEEP)
#define LIST_DEQUEUE(thr) ((thr)->sleep = ZEROTHR_AWAKE)
#include <zero/list.h>

static zerothrqueue thrsleepqueue;
THREADLOCAL zerothr thrself;

void
thrsleep1(zerothrqueue *queue)
{
    zerothr *thr = &thrself;

    if (!queue) {
        queue = &thrsleepqueue;
    }
    listpush(queue, thr);
    do {
        if (!m_atomread(thr->sleep)) {

            return;
        }
        thryield();
    } while (1);
             
    return;
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

#if 0 /* FIXME: move this to cond.c */
void
condwait(zerocond *cond)
{
    ;
}
#endif

