#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

#if (MJOL_VGA_TEXT)
extern void mjolinitvga(struct mjolgame *game);
#define MJOL_VGA_TEXT_INIT mjolinitvga
#else
#define MJOL_VGA_TEXT_INIT NULL
#endif
#if (MJOL_TTY)
extern void mjolinittty(struct mjolgame *game);
#define MJOL_TTY_INIT      mjolinittty
#else
#define MJOL_TTY_INIT      NULL
#endif
#if (MJOL_X11)
extern void mjolinitx11(struct mjolgame *game);
#define MJOL_X11_INIT      mjolinitx11
#else
#define MJOL_X11_INIT      NULL
#endif

struct mjolgamescr mjolgamescr;

typedef void mjolinitscrfunc(struct mjolgame *);
static mjolinitscrfunc *mjolinitscrfunctab[4] = {
    NULL,
    MJOL_VGA_TEXT_INIT,
    MJOL_TTY_INIT,
    MJOL_X11_INIT
};

void
mjolinitscr(struct mjolgame *game)
{
    mjolinitscrfunc *func = mjolinitscrfunctab[game->scrtype];

    if (func) {
        func(game);
    } else {
        fprintf(stderr, "no initializer for screen type\n");

        exit(1);
    }

    return;
}

