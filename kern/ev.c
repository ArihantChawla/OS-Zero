#include <kern/ev.h>
#include <zero/mtx.h>

/*
 * WARNING: this file has not been tested or debugged at all, so consider it
 * early pseudo-code. :)
 */

/* dequeue character from keyboard queue. FIXME: may not work */
unsigned char
evdeqkbdchar(struct evkbdqchar *queue)
{
    int32_t       n;
    int32_t       in;
    int32_t       out;
    unsigned char retval = 0;
    int           done = 0;

    mtxlk(&queue->lk);
    n = EVKBDQNCHAR;
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
            n = EVKBDQNCHAR;
            in = queue->in;
            if (in < out || in < n - 1) {
                retval = queue->ctab[in];
                in++;
                queue->in = in;
                done = 1;
            }
            mtxunlk(&queue->lk);
        } while (!done);
    }

    return retval;
};

/* queue character to keyboard queue. FIXME: may not work */
void
evqkbdchar(struct evkbdqchar *queue, unsigned char ch)
{
    int32_t n;
    int32_t in;
    int32_t out;
    int     done = 0;

    mtxlk(&queue->lk);
    n = EVKBDQNCHAR;
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
            n = EVKBDQNCHAR;
            in = queue->in;
            out = queue->out;
            if (out < n - 1 || out < in) {
                queue->ctab[out] = ch;
                out++;
                queue->out = out;
                done = 1;
            }
            mtxunlk(&queue->lk);
        } while (!done);
    }

    return;
}

