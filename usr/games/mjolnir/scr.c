#include <stdio.h>
#include <stdlib.h>
#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>
#include <mjolnir/scr.h>

#if (MJOL_VGA_TEXT)
extern void mjolinitvga(struct mjolgamedata *);
#define MJOL_VGA_TEXT_INIT mjolinitvga
#else
#define MJOL_VGA_TEXT_INIT NULL
#endif
#if (MJOL_TTY)
extern void mjolinittty(struct mjolgamedata *gamedata);
#define MJOL_TTY_INIT      mjolinittty
#else
#define MJOL_TTY_INIT      NULL
#endif
#if (MJOL_X11)
extern void mjolinitx11(struct mjolgamedata *);
#define MJOL_X11_INIT      mjolinitx11
#else
#define MJOL_X11_INIT      NULL
#endif

struct mjolgamescr mjolgamescr;

typedef void mjolinitscrfunc(struct mjolgamedata *);
static mjolinitscrfunc *mjolinitscrfunctab[4] = {
    NULL,
    MJOL_VGA_TEXT_INIT,
    MJOL_TTY_INIT,
    MJOL_X11_INIT
};

void
mjolinitscr(struct mjolgamedata *gamedata)
{
    mjolinitscrfunc *func = mjolinitscrfunctab[gamedata->scrtype];

    if (func) {
        func(gamedata);
    } else {
        fprintf(stderr, "no initializer for screen type\n");

        exit(1);
    }

    return;
}

