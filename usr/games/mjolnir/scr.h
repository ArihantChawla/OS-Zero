#ifndef __MJOLNIR_SCR_H__
#define __MJOLNIR_SCR_H__

#include <mjolnir/conf.h>
#include <mjolnir/mjol.h>

#if (MJOL_VGA_TEXT)
#include <mjolnir/vga.h>
#if (MJOL_USE_COLORS)
#endif
#endif
#if (MJOL_TTY)
#include <mjolnir/tty.h>
#endif
#if (MJOL_X11)
#define MJOL_USE_IMLIB2
#include <mjolnir/x11.h>
#endif

struct mjolgamescr {
    long   x;
    long   y;
    int  (*getch)(void);
    void (*drawchar)(struct mjolgamedata *, struct mjolchardata *);
    int  (*printmsg)(const char *fmt, ...);
    int  (*refresh)(void);
};

#endif /* __MJOLNIR_SCR_H__ */

