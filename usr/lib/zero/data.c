#if 0

#include <stdio.h>
#include <stdlib.h>
#include <zero/mtx.h>
#define HTLIST_TYPE  struct htlist
#define HTLIST_QTYPE struct queue
#include <zero/htlist.h>
#include <zero/hash.h>

struct htlist {
    long         key;
    struct htlist *prev;
    struct htlist *next;
};

struct queue {
    volatile long  lk;
    struct htlist   *head;
    struct htlist   *tail;
};

static struct htlist  ltab[64];
static struct queue q;

int
main(int argc, char *argv[])
{
    int          i;
    struct htlist *lp;

    if (argc > 1) {
        fprintf(stderr, "ACK! I don't need your damned arguments! :)\n");
        for (i = 0 ; i < argc ; i++) {
            fprintf(stderr, "%s\n", argv[i]);
        }
    }
    /* pop test */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        htlistpush(&q, &ltab[i]);
    }
    for (i = 0 ; i < 64 ; i++) {
        htlistpop(&q, &lp);
        if (lp) {
            printf("%ld\n", lp->key);
        }
    }
    htlistpop(&q, &lp);
    printf("%p\n", lp);

    /* deq test */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        htlistpush(&q, &ltab[i]);
    }
    for (i = 0 ; i < 64 ; i++) {
        htlistdeq(&q, &lp);
        if (lp) {
            printf("%ld\n", lp->key);
        }
    }
    htlistpop(&q, &lp);
    printf("%p\n", lp);

    /* rm test forward */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        htlistpush(&q, &ltab[i]);
    }
    for (i = 0 ; i < 64 ; i++) {
        htlistrm(&q, &ltab[i]);
        printf("%ld - %ld..%ld\n", ltab[i].key,
               (q.head) ? q.head->key : -1,
               (q.tail) ? q.tail->key : -1);
    }
    htlistpop(&q, &lp);
    printf("%p\n", lp);

    /* rm test backwards */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        htlistpush(&q, &ltab[i]);
    }
    for (i = 63 ; i >= 0 ; i--) {
        htlistrm(&q, &ltab[i]);
        printf("%ld - %ld..%ld\n", ltab[i].key,
               (q.head) ? q.head->key : -1,
               (q.tail) ? q.tail->key : -1);
    }
    htlistpop(&q, &lp);
    printf("%p\n", lp);

    exit(0);
}

#endif
