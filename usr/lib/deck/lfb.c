#include <features.h>
#include <stdlib.h>
#include <stdio.h>
//#include <unistd.h>
//#include <zero/deck.h>
#if (_ZERO_SOURCE)
#include <kern/syscall.h>
#endif

void *
deckmaplfb(struct deck *deck, long depth, long width, long height)
{
    void             *lfb;
    struct sysmemreg  memreg;
    long              pixsize;

    switch (depth) {
        case 8:
            pixsize = 1;

            break;
        case 15:
        case 16:
            pixsize = 2;

            break;
        case 24:
            pixsize = 3;

            break;
        case 32:
            pixsize = 4;

            break;
        default:
            fprintf(stderr, "DECK: unknown framebuffer depth\n");

            exit(1);
    }

    /* FIXME: fix the stuff below */
#if (_ZERO_SOURCE) && 0
    setperm(&memreg.perm, getuid(), getgid(), PERM_UW);
    memreg.adr = NULL;
    memreg.ofs = 0;
    memreg.len = pixsize * width * height;
    lfb = _syscall(SYS_MAP, SYS_NODESC, 0, &memreg);
#endif

    return lfb;
}

