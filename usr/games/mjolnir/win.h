#ifndef __MJOLNIR_WIN_H__
#define __MJOLNIR_WIN_H__

#include <mjolnir/conf.h>

#if (MJOLNIR_VGA_TEXT)
#include <mjolnir/vga.h>
#endif
#if (MJOLNIR_TTY)
#include <mjolnir/tty.h>
#endif
#if (MJOLNIR_X11)
#define MJOLNIR_USE_IMLIB2
#include <mjolnir/x11.h>
#endif

#endif /* __MJOLNIR_WIN_H__ */

