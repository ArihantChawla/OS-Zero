#ifndef __UI_SYS_XORG_H__
#define __UI_SYS_XORG_H__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

void uiinitapi_xorg(struct ui *ui);
void uiinit_xorg(struct ui *ui, int argc, char *argv[]);

#define UIXORGNDEFEV    LASTEvent
#define UIXORGROOTWINID None

struct uienv_xorg {
    Display      *display;
    Visual       *visual;
    Colormap      colormap;
    int           screen;
    int           depth;
    struct uiwin *mainwin;
    Atom          wmdelete;
};

#endif /* __UI_SYS_XORG_H__ */

