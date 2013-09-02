#include <stdlib.h>
#include <mjolnir/conf.h>

#if (MJOL_TTY)

#include <curses.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

void
mjoldrawchartty(struct mjolgame *game, struct mjolchar *data)
{
    ;
}

void
mjolclosetty(void)
{
    endwin();

    return;
}

void
mjolinittty(struct mjolgame *game)
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    game->scr = calloc(1, sizeof(struct mjolscr));
    game->scr->getch = getch;
    game->scr->drawchar = mjoldrawchartty;
    game->scr->printmsg = printw;
    game->scr->refresh = refresh;
    game->scr->close = mjolclosetty;

    return;
}

#endif /* MJOL_TTY */

