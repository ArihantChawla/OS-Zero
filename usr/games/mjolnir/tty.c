/* mjolopenwin(), mjolclosewin(), mjolttyexit(), mjoldraw(), mjoldrawscreen() */

#include <mjol/conf.h>

#if (MJOL_TTY_GRAPHICS)

#include <mjol/mjol.h>
#include <mjol/win.h>

void
mjolopentty(void)
{
    initscr();

    return;
}

void
mjolclosetty(void)
{
    endwin();

    return;
}

#endif /* MJOL_TTY_GRAPHICS */

