#ifndef __MJOLNIR_WIN_H__
#define __MJOLNIR_WIN_H__

#include <mjolnir/conf.h>

/* mjolopenwin(), mjolclosewin(), mjolttyexit(), mjoldraw, mjoldrawscreen */
#if (MJOLNIR_VGACON_GRAPHICS)
#include <mjolnir/vgacon.h>
#endif
#if (MJOLNIR_TTY_GRAPHICS)
/* curses */
#include <mjolnir/tty.h>
#endif
#if (MJOLNIR_X11_GRAPHICS)
#define MJOLNIR_USE_IMLIB2
#include <mjolnir/x11.h>
#endif

#endif /* __MJOLNIR_WIN_H__ */

