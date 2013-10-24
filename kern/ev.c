#include <kern/ev.h>
#include <zero/mtx.h>

/*
 * WARNING: this file has not been tested or debugged at all, so consider it
 * early pseudo-code. :)
 */

/* dequeue character from keyboard queue. FIXME: may not work */
static char
evdeqkbdchar(struct evkbdqchar *queue)
{
    int32_t       n;
    int32_t       cur;
    int32_t       ndx;
    unsigned char retval = 0;

    mtxlk(&queue->lk);
    n = queue->n;
    cur = queue->cur;
    ndx = queue->ndx;
    if (ndx) {
        if (cur != ndx) {
            if (cur < n - 1 && cur < ndx) {
                retval = queue->ctab[cur++];
                queue->cur = cur;
                mtxunlk(&queue->lk);
            } else {
                mtxunlk(&queue->lk);
                do {
                    mtxlk(&queue->lk);
                    n = queue->n;
                    cur = queue->cur;
                    ndx = queue->ndx;
                    if (cur == n - 1) {
                        cur = 0;
                    }
                    retval = queue->ctab[cur];
                    if (cur != ndx) {
                        cur++;
                        queue->cur = cur;
                    }
                    mtxunlk(&queue->lk);
                } while (cur == ndx);
            }
        }
    }

    return retval;
};

/* queue character to keyboard queue. FIXME: may not work */
static void
evqkbdchar(struct evkbdqchar *queue, unsigned char ch)
{
    int32_t n;
    int32_t cur;
    int32_t ndx;

    mtxlk(&queue->lk);
    n = queue->n;
    cur = queue->cur;
    ndx = queue->ndx;
    if (n) {
        if (ndx == n - 1) {
            ndx = 0;
        }
        if (ndx < n - 1 && ndx < cur) {
            queue->ctab[ndx++] = ch;
            queue->ndx = ndx;
            mtxunlk(&queue->lk);
        } else {
            mtxunlk(&queue->lk);
            do {
                mtxlk(&queue->lk);
                n = queue->n;
                cur = queue->cur;
                ndx = queue->ndx;
                if ((ndx < n - 1 && ndx > cur) || ndx < cur) {
                    queue->ctab[ndx++] = ch;
                    queue->ndx = ndx;
                }
                mtxunlk(&queue->lk);
            } while (ndx == cur);
        }
    }
}

