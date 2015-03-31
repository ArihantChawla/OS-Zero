#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

#if (MJOL_VGA_TEXT)
extern void mjolopenvga(struct mjolgame *game);
#define MJOL_VGA_TEXT_OPEN mjolopenvga
#else
#define MJOL_VGA_TEXT_OPEN NULL
#endif
#if (MJOL_TTY)
extern void mjolopentty(struct mjolgame *game);
#define MJOL_TTY_OPEN      mjolopentty
#else
#define MJOL_TTY_OPEN      NULL
#endif
#if (MJOL_X11)
extern void mjolopenx11(struct mjolgame *game);
#define MJOL_X11_OPEN      mjolopenx11
#else
#define MJOL_X11_OPEN      NULL
#endif

typedef void mjolopenscrfunc(struct mjolgame *);
static mjolopenscrfunc *mjolopenscrfunctab[4] = {
    NULL,
    MJOL_VGA_TEXT_OPEN,
    MJOL_TTY_OPEN,
    MJOL_X11_OPEN
};

void
mjolopenscr(struct mjolgame *game)
{
    mjolopenscrfunc *func = mjolopenscrfunctab[game->scrtype];

    if (func) {
        func(game);
    } else {
        fprintf(stderr, "no initialiser for screen type\n");

        exit(1);
    }

    return;
}

