#if 0

#include <stdio.h>
#include <stdlib.h>
#include <zero/mtx.h>
#define LIST_TYPE  struct list
#define LIST_QTYPE struct queue
#include <zero/list.h>
#include <zero/hash.h>

struct list {
    long         key;
    struct list *prev;
    struct list *next;
};

struct queue {
    volatile long  lk;
    struct list   *head;
    struct list   *tail;
};

static struct list  ltab[64];
static struct queue q;

int
main(int argc, char *argv[])
{
    int          i;
    struct list *lp;

    if (argc > 1) {
        fprintf(stderr, "ACK! I don't need your damned arguments! :)\n");
        for (i = 0 ; i < argc ; i++) {
            fprintf(stderr, "%s\n", argv[i]);
        }
    }
    /* pop test */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        listpush(&q, &ltab[i]);
    }
    for (i = 0 ; i < 64 ; i++) {
        listpop(&q, &lp);
        if (lp) {
            printf("%ld\n", lp->key);
        }
    }
    listpop(&q, &lp);
    printf("%p\n", lp);

    /* deq test */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        listpush(&q, &ltab[i]);
    }
    for (i = 0 ; i < 64 ; i++) {
        listdeq(&q, &lp);
        if (lp) {
            printf("%ld\n", lp->key);
        }
    }
    listpop(&q, &lp);
    printf("%p\n", lp);

    /* rm test forward */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        listpush(&q, &ltab[i]);
    }
    for (i = 0 ; i < 64 ; i++) {
        listrm(&q, &ltab[i]);
        printf("%ld - %ld..%ld\n", ltab[i].key,
               (q.head) ? q.head->key : -1,
               (q.tail) ? q.tail->key : -1);
    }
    listpop(&q, &lp);
    printf("%p\n", lp);

    /* rm test backwards */
    for (i = 0 ; i < 64 ; i++) {
        ltab[i].key = i;
        listpush(&q, &ltab[i]);
    }
    for (i = 63 ; i >= 0 ; i--) {
        listrm(&q, &ltab[i]);
        printf("%ld - %ld..%ld\n", ltab[i].key,
               (q.head) ? q.head->key : -1,
               (q.tail) ? q.tail->key : -1);
    }
    listpop(&q, &lp);
    printf("%p\n", lp);

    exit(0);
}

#endif
