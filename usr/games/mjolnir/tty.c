#include <stdlib.h>
#include <mjolnir/conf.h>

#if (MJOL_TTY)

#include <curses.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

void
mjolmovetotty(int x, int y)
{
    ;
}

void
mjoldrawchrtty(struct mjolgame *game, struct mjolchr *data)
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
mjolmkscrtty(struct mjolgame *game)
{
    struct mjolwindatatty *data = calloc(1, sizeof(struct mjolwindatatty));
    WINDOW                *win;

    /* create main window */
    win = newwin(0, 0, 0, 0);
    if (!win) {
        mjolclosetty();
        fprintf(stderr, "window creation failure\n");
        
        exit(1);
    }
    data->mainwin = win;
    /* create message window */
    win = subwin(data->mainwin, 1, 0, 0, 0);
    if (!win) {
        mjolclosetty();
        fprintf(stderr, "window creation failure\n");
        
        exit(1);
    }
    data->msgwin = win;
    /* create status window */
    win = subwin(data->mainwin, 1, 0, game->height - 1, 0);
    if (!win) {
        mjolclosetty();
        fprintf(stderr, "window creation failure\n");
        
        exit(1);
    }
    data->statwin = win;
    /* create game window */
    win = subwin(data->mainwin, game->height - 2, 0, 1, 0);
    if (!win) {
        mjolclosetty();
        fprintf(stderr, "window creation failure\n");
        
        exit(1);
    }
    data->gamewin = win;
    game->scr = calloc(1, sizeof(struct mjolscr));
    if (game->scr) {
        game->scr->data = data;
        game->scr->getch = getch;
        game->scr->moveto = mjolmovetotty;
        game->scr->drawchr = mjoldrawchrtty;
        game->scr->printmsg = printw;
        game->scr->refresh = refresh;
        game->scr->close = mjolclosetty;
    } else {
        mjolclosetty();
        fprintf(stderr, "memory allocation failure\n");
        
        exit(1);
    }

    return;
}

void
mjolopentty(struct mjolgame *game)
{
    initscr();
    cbreak();
    keypad(stdscr, TRUE);
    noecho();
    mjolmkscrtty(game);
        
    return;
}

#endif /* MJOL_TTY */

