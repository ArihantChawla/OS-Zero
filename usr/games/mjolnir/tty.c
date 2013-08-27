#include <stdlib.h>
#include <mjolnir/conf.h>

#if (MJOL_TTY)

#include <curses.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

void
mjoldrawchartty(struct mjolgamedata *gamedata, struct mjolchardata *chardata)
{
    ;
}

void
mjolinittty(struct mjolgamedata *gamedata)
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    gamedata->scr = calloc(1, sizeof(struct mjolgamescr));
    gamedata->scr->getch = getch;
    gamedata->scr->drawchar = mjoldrawchartty;
    gamedata->scr->printmsg = printw;
    gamedata->scr->refresh = refresh;

    return;
}

void
mjolclosetty(void)
{
    endwin();

    return;
}

#endif /* MJOL_TTY */

