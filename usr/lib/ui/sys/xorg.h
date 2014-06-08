#ifndef __UI_SYS_XORG_H__
#define __UI_SYS_XORG_H__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

#define UIX11NDEFEV    LASTEvent
#define UIX11ROOTWINID None

struct uienv_x11 {
    Display      *display;
    Visual       *visual;
    Colormap      colormap;
    int           screen;
    int           depth;
    struct uiwin *mainwin;
    Atom          wmdelete;
};

#endif /* __UI_SYS_XORG_H__ */

