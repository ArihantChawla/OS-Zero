#ifndef __MJOLNIR_TTY_H__
#define __MJOLNIR_TTY_H__

#include <mjolnir/conf.h>

#if (MJOL_TTY)

#include <curses.h>

struct mjolwindatatty {
    void *mainwin;
    void *msgwin;
    void *statwin;
    void *gamewin;
};

#endif /* MJOL_TTY */

#endif /* __MJOLNIR_TTY_H__ */

