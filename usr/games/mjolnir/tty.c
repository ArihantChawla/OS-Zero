/* mjolopenwin(), mjolclosewin(), mjolttyexit(), mjoldraw(), mjoldrawscreen() */

#include <mjolnir/conf.h>

#if (MJOL_TTY)

#include <mjolnir/mjol.h>
#include <mjolnir/win.h>

void
mjolopentty(void)
{
    initscr();
    raw();
    keypad(stdscr, TRUE);
    noecho();

    return;
}

void
mjolclosetty(void)
{
    endwin();

    return;
}

#endif /* MJOL_TTY */

