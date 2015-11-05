#include <mjolnir/conf.h>

#include <X11/Xlibint.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/Xlocale.h>
#include <X11/cursorfont.h>
#include <X11/keysym.h>

#if (MJOL_X11)

struct mjolscrx11 {
    Display *disp;
    Window  *gamewin;
    Window  *msgwin;
    Window  *statwin;
};

static struct mjolscrx11 mjolscrx11;

int
mjolgetch(void)
{
    XEvent ev;
    int    retval;

    XNextEvent(mjolscrx11.disp, &ev);
    if (XFilterEvent(&ev, None)) {
        XUngrabKeyboard(mjolscrx11.disp, CurrentTime);
    } else {
        /* FIXME: do the stuff */
    }

    return retval;
}

#endif

