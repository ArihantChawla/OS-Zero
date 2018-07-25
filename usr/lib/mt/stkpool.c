#include <stdio.h>
#include <stdlib.h>
#include <mach/asm.h>
#define STKPOOL_LOCKFREE 1
#include <mt/stkpool.h>
#include <pthread.h>

#define STKPOOL_THREADS 8

static void                **ptrtab[STKPOOL_THREADS];
static volatile m_atomic_t   thrcnt;
pthread_t                    thrtab[STKPOOL_THREADS];

void *
thrstart(void *arg)
{
    m_atomic_t      id = m_fetchadd(&thrcnt, 1);
    struct lkpool  *pool = arg;
    void          **pptr = ptrtab[id];
    long            ndx;
    void            *ptr;

    fprintf(stderr, "thrstart(%lx)\n", id);
    lkpoolinit(pool);
    for (ndx = 0 ; ndx < 1048576 ; ndx++) {
        ptr = &pptr[ndx];
        pptr[ndx] = ptr;
    }
    for (ndx = 0 ; ndx < 1048576 ; ndx++) {
        if (!stkpoolpushptr(pool, ptrtab[ndx])) {

            exit(1);
        }
    }
    for (ndx = 0 ; ndx < 1048576 ; ndx++) {
        if (!stkpoolpop(pool)) {

            exit(1);
        }
    }

    return arg;
}

int
main(int argc, char *argv[])
{
    struct lkpool  pool[STKPOOL_THREADS];
    long           ndx;
    int            res;
    void          *ptr;

    for (ndx = 0 ; ndx < STKPOOL_THREADS ; ndx++) {
        ptrtab[ndx] = malloc(1048576 * STKPOOL_THREADS * sizeof(void *));
        if (!ptrtab[ndx]) {

            exit(1);
        }
    }
    for (ndx = 0 ; ndx < STKPOOL_THREADS ; ndx++) {
        res = pthread_create(&thrtab[ndx], NULL, thrstart, &pool[ndx]);
        if (res) {
            fprintf(stderr, "pthread_create() FAILED\n");

            exit(1);
        }
    }
    for (ndx = 0 ; ndx < STKPOOL_THREADS ; ndx++) {
        res = pthread_join(thrtab[ndx], &ptr);
        if (res) {
            fprintf(stderr, "pthread_join() FAILED\n");

            exit(1);
        }
    }

    exit(0);
}

