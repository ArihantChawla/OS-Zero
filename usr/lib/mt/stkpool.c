#include <stdio.h>
#include <stdlib.h>
#include <mach/asm.h>
#define STKPOOL_LOCKFREE 1
#include <mt/stkpool.h>
#include <pthread.h>

#define STKPOOL_THREADS         8
#define STKPOOL_THREAD_POINTERS 16777216

static struct lkpool         pool[STKPOOL_THREADS];
static void                **ptrtab[STKPOOL_THREADS];
pthread_t                    thrtab[STKPOOL_THREADS];
static volatile m_atomic_t   thrcnt;

void *
thrstart(void *arg)
{
    m_atomic_t      id = m_fetchadd(&thrcnt, 1);
    void          **pptr = ptrtab[id];
    long            ndx;
    void            *ptr;

    fprintf(stderr, "thrstart(%lx)\n", id);
    lkpoolinit(pool);
    for (ndx = 0 ; ndx < STKPOOL_THREAD_POINTERS ; ndx++) {
        ptr = &pptr[ndx];
        pptr[ndx] = ptr;
    }
    pptr = ptrtab[id];
    for (ndx = 0 ; ndx < STKPOOL_THREAD_POINTERS ; ndx++) {
        if (!stkpoolpushptr(pool, pptr[ndx])) {
            fprintf(stderr, "stkpoolpushptr() failed\n");

            exit(1);
        }
    }
    for (ndx = 0 ; ndx < STKPOOL_THREAD_POINTERS ; ndx++) {
        if (!stkpoolpop(pool)) {
            fprintf(stderr, "stkpoolpop() failed\n");

            exit(1);
        }
    }

    return arg;
}

int
main(int argc, char *argv[])
{
#if 0
    long           ndx;
    int            res;
    void          *ptr;
#endif

    ptrtab[0] = malloc(STKPOOL_THREAD_POINTERS * sizeof(void *));
    thrstart(&pool[0]);
#if 0
    for (ndx = 0 ; ndx < STKPOOL_THREADS ; ndx++) {
        ptrtab[ndx] = malloc(STKPOOL_THREAD_POINTERS * sizeof(void *));
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
#endif

    exit(0);
}

