#include <kern/ev.h>
#include <zero/mtx.h>

/*
 * WARNING: this file has not been tested or debugged at all, so consider it
 * early pseudo-code. :)
 */

/* wait for event of type on kernel data structure wadr */
void
evwait(void *wadr, long type)
{
    ;
}

/* awaken threads waiting for event type on kernel data structure wadr */
void
evpost(void *wadr, long type)
{
    ;
}

#if 0

/* dequeue character from keyboard queue. FIXME: may not work */
unsigned char
evdeqkbdchar(struct evkbdqchar *queue)
{
    long          n = EVKBDQNCHAR;
    long          in;
    long          out;
    unsigned char retval = 0;
    int           done = 0;

    mtxlk(&queue->lk);
    in = queue->in;
    out = queue->out;
    if (in == n - 1) {
        in = 0;
        queue->in = in;
    }
    if (in < out) {
        retval = queue->ctab[in];
        in++;
        queue->in = in;
        mtxunlk(&queue->lk);
    } else {
        mtxunlk(&queue->lk);
        do {
            mtxlk(&queue->lk);
            in = queue->in;
            if (in < out || in < n - 1) {
                retval = queue->ctab[in];
                in++;
                queue->in = in;
                done = 1;
            }
            mtxunlk(&queue->lk);
            if (!done) {
                evwait(queue, EVQUEUE);
            } else {
                evpost(queue, EVDEQUEUE);
            }
        } while (!done);
    }

    return retval;
};

/* queue character to keyboard queue. FIXME: may not work */
void
evqkbdchar(struct evkbdqchar *queue, unsigned char ch)
{
    long n = EVKBDQNCHAR;
    long in;
    long out;
    int  done = 0;

    mtxlk(&queue->lk);
    in = queue->in;
    out = queue->out;
    if (out == n - 1) {
        out = 0;
    }
    if (out < n - 1 && out < in) {
        queue->ctab[out] = ch;
        out++;
        queue->out = out;
        mtxunlk(&queue->lk);
    } else {
        mtxunlk(&queue->lk);
        do {
            mtxlk(&queue->lk);
            in = queue->in;
            out = queue->out;
            if (out < n - 1 || out < in) {
                queue->ctab[out] = ch;
                out++;
                queue->out = out;
                done = 1;
            }
            mtxunlk(&queue->lk);
            if (!done) {
                evwait(queue, EVDEQUEUE);
            } else {
                evpost(queue, EVQUEUE);
            }
        } while (!done);
    }

    return;
}

#endif /* 0 */

