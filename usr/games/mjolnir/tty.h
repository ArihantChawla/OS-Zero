#ifndef __MJOLNIR_TTY_H__
#define __MJOLNIR_TTY_H__

#include <mjolnir/conf.h>

#if (MJOLNIR_TTY)

#include <curses.h>

#define mjolgetch    getch
#define mjolprintmsg printw
#define mjolrefscr   refresh

#endif /* MJOLNIR_TTY */

#endif /* __MJOLNIR_TTY_H__ */

