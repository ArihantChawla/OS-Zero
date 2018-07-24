#include <stdio.h>
#include <stdlib.h>
#define STKPOOL_LOCKFREE 1
#include <mt/stkpool.h>

int
main(int argc, char *argv[])
{
    void          **ptrtab = malloc(16777216 * sizeof(void *));
    struct lkpool   pool;
    long            ofs;
    long            ndx;
    uintptr_t       uptr;

    lkpoolinit(&pool);
    for (ndx = 0 ; ndx < 16777216 ; ndx++) {
        uptr = qrand32();
        ptrtab[ndx] = (void *)tmhash64(uptr);
    }
    for (ndx = 0 ; ndx < 16777216 ; ndx++) {
        if (!stkpoolpushptr(&pool, ptrtab[ndx])) {

            exit(1);
        }
    }
    for (ndx = 0 ; ndx < 16777216 ; ndx++) {
        if (!stkpoolpop(&pool)) {

            exit(1);
        }
    }

    exit(1);
}

