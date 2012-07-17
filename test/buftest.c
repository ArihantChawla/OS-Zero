#include <stdio.h>
#include <stdlib.h>
#include <zero/mtx.h>
#define BUFTEST 1
#include "buf.h"

#define devgetblk(buf, blk) devfindblk(buf, blk, 0)

int
main(int argc,
     char *argv[])
{
    struct devbuf db = { MTXINITVAL, NULL };
    struct buftab buf = { (void *)0xb4b4b4b4, 0 };
    
    fprintf(stderr, "LVLBITS: %ld:%ld:%ld:%ld\n", NLVL0BIT, NLVL1BIT, NLVL2BIT, NLVL3BIT);
    devbufblk(&db, UINT64_C(0), &buf);
    fprintf(stderr, "RET: %p (%p)\n", devgetblk(&db, UINT64_C(0)), &buf);
    devbufblk(&db, UINT64_C(0xffff), &buf);
    fprintf(stderr, "RET: %p (%p)\n", devgetblk(&db, UINT64_C(0xffff)), &buf);
    devbufblk(&db, UINT64_C(0xffffffff), &buf);
    fprintf(stderr, "RET: %p (%p)\n", devgetblk(&db, UINT64_C(0xffffffff)), &buf);
    devbufblk(&db, UINT64_C(0xffffffffffff), &buf);
    fprintf(stderr, "RET: %p (%p)\n", devgetblk(&db, UINT64_C(0xffffffffffff)), &buf);
#if (NBUFBLKBIT == 64)
    devbufblk(&db, UINT64_C(0xffffffffffffffff), &buf);
    fprintf(stderr, "RET: %p (%p)\n", devgetblk(&db, UINT64_C(0xffffffffffffffff)), &buf);
#endif

#if (NBUFBLKBIT == 64)
    devfreeblk(&db, UINT64_C(0xffffffffffffffff));
#endif
    devfreeblk(&db, UINT64_C(0xffffffffffff));
    devfreeblk(&db, UINT64_C(0xffffffff));
    devfreeblk(&db, UINT64_C(0xffff));
    devfreeblk(&db, UINT64_C(0));
    fprintf(stderr, "RET: %p (%p)\n", devgetblk(&db, UINT64_C(0xffffffffffff)), &buf);

    return 0;
}

