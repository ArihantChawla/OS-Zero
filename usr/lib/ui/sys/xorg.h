#ifndef __UI_SYS_XORG_H__
#define __UI_SYS_XORG_H__

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysymdef.h>

void uiinit_xorg(struct ui *ui, char argc, char *argv[]);

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

typedef void * xorginitcolors_t(void *, int32_t *, size_t);
struct uiapi_xorg {
    xorginitcolors_t *initcolors;
};

#endif /* __UI_SYS_XORG_H__ */

