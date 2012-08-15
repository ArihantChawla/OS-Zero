#if (ZPCX11)

/* TODO: add buttons '<', '>', and 'u' */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <zpc/zpc.h>
#include <zpc/op.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include "x11.h"

#define HOVERBUTTONS 0

#define ZPC_TITLE "Zero Programmer's Calculator"

void imlib2init(struct x11app *app);
Pixmap imlib2loadimage(struct x11app *app, const char *filename, int w, int h);

void stkenterinput(void);
void stkqueueinput(const char *str);

void zpcdel(void);
void zpccls(void);
void zpcclrstk(void);
void zpcevaltop(void);
void zpcenter(void);
void zpcquit(void);

extern struct zpctoken   *zpcregstk[];
extern struct zpcstkitem *zpcinputitem;

#define NHASHITEM 1024
static struct x11wininfo *winhash[NHASHITEM] ALIGNED(PAGESIZE);
static zpccop_t          *buttonopertab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, not64, shr64, inc64, NULL, NULL, NULL },
    { NULL, NULL, NULL, mod64, shrl64, dec64, NULL, NULL, NULL },
    { NULL, NULL, NULL, div64, shl64, ror64, NULL, NULL, NULL },
    { NULL, NULL, NULL, mul64, xor64, rol64, NULL, NULL, NULL },
    { NULL, NULL, NULL, sub64, or64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, add64, and64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
#if 0
static zpcfop_t        *buttonopertabdbl[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fdiv64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, mul64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fsub64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fadd64, NULL, NULL, NULL},
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
#endif
static Window             buttonwintab[ZPC_NROW][ZPC_NCOLUMN];
static uint8_t            buttontypetab[ZPC_NROW][ZPC_NCOLUMN] =
{
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL },
    { ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL, ZPCBUTTONUTIL }
};
static const char        *buttonstrtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { "d", "e", "f", "~", ">>>", "++", "(", "QUIT", NULL },
    { "a", "b", "c", "%", ">>", "--", ")", "CLR", NULL },
    { "7", "8", "9", "/", "<<", "..>", "0x", "COPY", NULL },
    { "4", "5", "6", "*", "^", "<..", "0b", "POP", NULL },
    { "1", "2", "3", "-", "|", "=", "F", "DUP", NULL },
    { "0", NULL, ",", "+", "&", NULL, "U", "DROP", NULL }, // TODO: 2nd is "."
    { " ", NULL, NULL, "<", ">", "[", "]", "ENTER", NULL }
};
static const char        *buttonlabeltab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { "d", "e", "f", "~", ">>>", "++", "(", "QUIT", "EQU" },
    { "a", "b", "c", "%", ">>", "--", ")", "CLR", "VEC" },
    { "7", "8", "9", "/", "<<", "..>", "0x", "COPY", "MAT" },
    { "4", "5", "6", "*", "^", "<..", "0b", "POP", "PLOT" },
    { "1", "2", "3", "-", "|", "=", "F", "DUP", "RAD" },
    { "0", ".", ",", "+", "&", "EVAL", "U", "DROP", "UNIT" },
    { "SPACE", "DEL", "<", ">", "[", "]", "I", "CLS", "ENTER" }
};
static const char        *buttontop1tab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, "not", "shra", "inc", NULL, NULL, NULL },
    { NULL, NULL, NULL, "mod", "shr", "dec", NULL, NULL, NULL },
    { NULL, NULL, NULL, "div", "shl", "ror", NULL, NULL, NULL },
    { NULL, NULL, NULL, "mul", "xor", "rol", NULL, NULL, NULL },
    { NULL, NULL, NULL, "sub", "or", NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "add", "and", NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
static const char        *buttontop2tab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fdiv", NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fmul", NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fsub", NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fadd", NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }
};
#define QUIT  0xff
#define ENTER 0xfe
#define EVAL  0xfd
#define CLR   0xfc
#define COPY  0xfb
#define POP   0xfa
#define DUP   0xf9
#define DROP  0xf8
#define DEL   0xf7
#define CLS   0xf6
#define EQU   0xf5
#define VEC   0xf4
#define MAT   0xf3
#define PLOT  0xf2
#define RAD   0xf1
#define UNIT  0xf0
#define LAST  0xf0
#define isaction(i)                                                     \
    ((i) >= UNIT && (i) <= QUIT)
#define toaction(i)                                                     \
    ((i) - LAST)
static const uint8_t      parmtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { 0, 0, 0, 1, 2, 1, 0, QUIT },
    { 0, 0, 0, 2, 2, 1, 0, CLR },
    { 0, 0, 0, 2, 2, 2, 0, COPY },
    { 0, 0, 0, 2, 2, 2, 0, POP },
    { 0, 0, 0, 2, 2, 0, 0, DUP },
    { 0, 0, 0, 2, 2, EVAL, 0, DROP },
    { 0, DEL, 0, 0, 0, 0, 0, CLS, ENTER }
};
static const uint8_t      isflttab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0, 0, 0 }
};
static zpcaction_t *actiontab[0xff - LAST + 1] =
{
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    zpccls,
    zpcdel,
    NULL,
    NULL,
    NULL,
    NULL,
    zpcclrstk,
    zpcevaltop,
    zpcenter,
    zpcquit
};
static char *zpcregstrtab[NREGSTK]
= {
    "ST0",
    "ST1",
    "ST2",
    "ST3",
    "ST4",
    "ST5",
    "ST6",
    "ST7",
    "ST8",
    "ST9",
    "ST10",
    "ST11",
    "ST12",
    "ST13",
    "ST14",
    "ST15"
};
#define BUTTONNORMAL  0
#define BUTTONHOVER   1
#define BUTTONCLICKED 2
#define NBUTTONSTATE  3
static Window         stkwintab[NREGSTK];
static Window         zpcregwintab[NREGSTK];
static Pixmap         buttonpmaps[NBUTTONSTATE];
static Pixmap         smallbuttonpmaps[NBUTTONSTATE];
XFontStruct          *font;
int                   fontw;
int                   fonth;
static struct x11app *app;
static Window         mainwin;
static Window         inputwin;
GC                    textgc;
GC                    asmtextgc;
GC                    numtextgc;
GC                    optextgc;
GC                    utiltextgc;
Atom                  wmdelete;

void
x11drawdisp(void)
{
    const char        *str;
    struct zpctoken   *token;
    struct zpcstkitem *item;
    int                i;
    int                len;
    Window             win;
    int                x;

    i = NSTKREG;
    while (i--) {
        token = zpcregstk[i];
        win = stkwintab[i];
        x = 8;
        XClearWindow(app->display, win);
        while ((token) && (token->str)) {
            len = strlen(token->str);
            XDrawString(app->display, win, textgc,
                        x,
                        (fonth >> 1) + 8,
                        token->str, len);
            x += len * fonth;
            token = token->next;
        }
        str = zpcregstrtab[i];
        win = zpcregwintab[i];
        x = 8;
        len = strlen(str);
        XDrawString(app->display, win, asmtextgc,
                    x,
                    (fonth >> 1) + 8,
                    str, len);
    }
    x = 8;
    item = zpcinputitem;
    len = strlen(item->str);
    win = inputwin;
    XClearWindow(app->display, win);
    if (item->scur != item->str) {
        XDrawString(app->display, win, textgc,
                    x,
                    (fonth + 8) >> 1,
                    item->str, len);
    }

    return;
}

void
x11initwm(void)
{
    wmdelete = XInternAtom(app->display,
                           "WM_DELETE_WINDOW",
                           False);
    XSetWMProtocols(app->display,
                    mainwin,
                    &wmdelete,
                    1);

    return;
}

void
x11inittitle(void)
{
    XTextProperty titleprop;
    char          *str = ZPC_TITLE;

    XStringListToTextProperty(&str, 1, &titleprop);
    XSetWMName(app->display, mainwin, &titleprop);
    XFree(titleprop.value);

    return;
}

void
x11initgcs(void)
{
    XGCValues gcval;
    XColor    color;

#if (ZPCREVERSE)
    gcval.foreground = WhitePixel(app->display, DefaultScreen(app->display));
#else
    gcval.foreground = BlackPixel(app->display, DefaultScreen(app->display));
#endif
    gcval.font = font->fid;
    gcval.graphics_exposures = False;
    textgc = XCreateGC(app->display, app->win,
                       GCForeground | GCFont | GCGraphicsExposures,
                       &gcval);
    if (!textgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    numtextgc = XCreateGC(app->display, app->win,
                          GCForeground | GCFont | GCGraphicsExposures,
                          &gcval);
    if (!numtextgc) {
        fprintf(stderr, "failed to create GC\n");
        
        exit(1);
    }
    if (!XParseColor(app->display,
                     app->colormap,
                     "orange",
                     &color)) {
        fprintf(stderr, "failed to parse color 'orange'\n");

        exit(1);
    }
    if (!XAllocColor(app->display,
                     app->colormap,
                     &color)) {
        fprintf(stderr, "failed to parse color 'orange'\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    asmtextgc = XCreateGC(app->display, app->win,
                       GCForeground | GCFont | GCGraphicsExposures,
                       &gcval);
    if (!asmtextgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    if (!XParseColor(app->display,
                     app->colormap,
                     "green",
                     &color)) {
        fprintf(stderr, "failed to parse color 'green'\n");

        exit(1);
    }
    if (!XAllocColor(app->display,
                     app->colormap,
                     &color)) {
        fprintf(stderr, "failed to parse color 'green'\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    optextgc = XCreateGC(app->display, app->win,
                         GCForeground | GCFont | GCGraphicsExposures,
                         &gcval);
    if (!optextgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }
    if (!XParseColor(app->display,
                     app->colormap,
                     "light blue",
                     &color)) {
        fprintf(stderr, "failed to parse color 'light blue'\n");

        exit(1);
    }
    if (!XAllocColor(app->display,
                     app->colormap,
                     &color)) {
        fprintf(stderr, "failed to parse color 'light blue'\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    utiltextgc = XCreateGC(app->display, app->win,
                          GCForeground | GCFont | GCGraphicsExposures,
                          &gcval);
    if (!utiltextgc) {
        fprintf(stderr, "failed to create GC\n");

        exit(1);
    }

    return;
}

XFontStruct *
x11loadfont(const char *fontname)
{
    XFontStruct *newfont;

    newfont = XLoadQueryFont(app->display, fontname);
    if (newfont) {
        fontw = newfont->max_bounds.width;
        fonth = newfont->ascent + newfont->descent;
    }

    return newfont;
}

void
x11addwininfo(struct x11wininfo *wininfo)
{
    long key = wininfo->id & (NHASHITEM - 1);

    wininfo->next = winhash[key];
    winhash[key] = wininfo;

    return;
}

struct x11wininfo *
x11findwininfo(Window id)
{
    struct x11wininfo *hashwin = NULL;
    long               key = id & (NHASHITEM - 1);

    hashwin = winhash[key];
    while ((hashwin) && hashwin->id != id) {
        hashwin = hashwin->next;
    }

    return hashwin;
}

#if (ZPCIMLIB2)
void
x11initpmaps(void)
{
    buttonpmaps[BUTTONNORMAL] = imlib2loadimage(app, "button.png",
                                                ZPC_BUTTON_WIDTH,
                                                ZPC_BUTTON_HEIGHT);
    smallbuttonpmaps[BUTTONNORMAL] = imlib2loadimage(app, "buttonsmall.png",
                                                     ZPC_SMALL_BUTTON_WIDTH,
                                                     ZPC_SMALL_BUTTON_HEIGHT);
#if (HOVERBUTTONS)
    buttonpmaps[BUTTONHOVER] = imlib2loadimage(app, "buttonhilited.png",
                                               ZPC_BUTTON_WIDTH,
                                               ZPC_BUTTON_HEIGHT);
#else
    buttonpmaps[BUTTONHOVER] = buttonpmaps[BUTTONNORMAL];
#endif
    buttonpmaps[BUTTONCLICKED] = buttonpmaps[BUTTONNORMAL];
#if 0
    buttonpmaps[BUTTONHOVER] = imlib2loadimage(app, "button.png",
                                               ZPC_BUTTON_WIDTH,
                                               ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONCLICKED] = imlib2loadimage(app, "button.png",
                                                 ZPC_BUTTON_WIDTH,
                                                 ZPC_BUTTON_HEIGHT);
#endif
#if 0
    buttonpmaps[BUTTONNORMAL] = imlib2loadimage("mosaic.png",
                                                ZPC_BUTTON_WIDTH,
                                                ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONHOVER] = imlib2loadimage("gaussian.png",
                                               ZPC_BUTTON_WIDTH,
                                               ZPC_BUTTON_HEIGHT);
#if 0
    buttonpmaps[BUTTONCLICKED] = imlib2loadimage("button_clicked.png",
                                                 ZPC_BUTTON_WIDTH,
                                                 ZPC_BUTTON_HEIGHT);
#endif
#endif
    smallbuttonpmaps[BUTTONHOVER] = smallbuttonpmaps[BUTTONNORMAL];
    smallbuttonpmaps[BUTTONCLICKED] = smallbuttonpmaps[BUTTONNORMAL];

    return;
}
#endif

struct x11app *
x11initapp(char *displayname)
{
    struct x11app *app;
    Display       *disp;

    app = calloc(1, sizeof(struct x11app));
    XInitThreads();
    disp = XOpenDisplay(displayname);
    if (!disp) {
        fprintf(stderr, "failed to open display\n");

        exit(1);
    }
    app->display = disp;
    app->screen = DefaultScreen(disp);
    app->depth = DefaultDepth(disp, app->screen);
    app->visual = DefaultVisual(disp, app->screen);
    app->colormap = DefaultColormap(disp, app->screen);

    return app;
}

Window
x11initwin(struct x11app *app, Window parent, int x, int y, int w, int h,
           int rvid)
{
    XSetWindowAttributes atr;
    Window               win;

    memset(&atr, 0, sizeof(atr));
    if (!parent) {
        parent = RootWindow(app->display, app->screen);
    }
    if (rvid) {
        atr.background_pixel = BlackPixel(app->display, app->screen);
    } else {
        atr.background_pixel = WhitePixel(app->display, app->screen);
    }
    win = XCreateWindow(app->display,
                        parent,
                        x,
                        y,
                        w,
                        h,
                        0,
                        app->depth,
                        InputOutput,
                        app->visual,
                        CWBackPixel,
                        &atr);

    return win;
}

void
displayexpose(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;
    struct zpctoken   *token = zpcregstk[wininfo->num];
    int                x;
    int                len;

    if (!event->xexpose.count) {
        x = 8;
        XClearWindow(app->display, wininfo->id);
        while ((token) && (token->str)) {
            len = strlen(token->str);
            XDrawString(app->display, wininfo->id, textgc,
                        x,
                        (fonth + 8) >> 1,
                        token->str, len);
            x += len * fonth;
            token = token->next;
        }
    }

    return;
}

void
labelexpose(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;
//    struct zpctoken   *token = zpcreglabel[wininfo->num];
    char              *str = zpcregstrtab[wininfo->num];
    int                x;
    int                len;

    if (!event->xexpose.count) {
        x = 8;
        XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                   smallbuttonpmaps[BUTTONNORMAL]);
        XClearWindow(app->display, wininfo->id);
        len = strlen(str);
        XDrawString(app->display, wininfo->id, asmtextgc,
                    x,
                    (fonth >> 1) + 8,
                    str, len);
        XSync(app->display, False);
    }

    return;
}

void
buttonexpose(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;
    size_t             len;

    if (!event->xexpose.count) {
        if (buttonpmaps[BUTTONNORMAL]) {
            XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                       buttonpmaps[BUTTONNORMAL]);
        }
        XClearWindow(app->display, wininfo->id);
    } else {
        XClearWindow(app->display, wininfo->id);
    }
    if (wininfo->str) {
        if (wininfo->topstr2) {
            len = strlen(wininfo->topstr2);
            XDrawString(app->display, wininfo->id, asmtextgc,
                        ZPC_BUTTON_WIDTH - len * fontw - 4,
//                        ZPC_BUTTON_HEIGHT - fonth + 8,
                        fonth + 4,
                        wininfo->topstr2, len);
            if (wininfo->topstr1) {
                len = strlen(wininfo->topstr1);
                XDrawString(app->display, wininfo->id, asmtextgc,
                            4,
//                            ZPC_BUTTON_HEIGHT - fonth + 8,
                            fonth + 4,
                            wininfo->topstr1, len);
            }
        } else if (wininfo->topstr1) {
            len = strlen(wininfo->topstr1);
            if (len) {
                XDrawString(app->display, wininfo->id, asmtextgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
//                            ZPC_BUTTON_HEIGHT - fonth + 8,
                            fonth + 4,
                            wininfo->topstr1, len);
            }
        }
        len = strlen(wininfo->str);
        if (len) {
            if (wininfo->topstr1) {
                XDrawString(app->display, wininfo->id, wininfo->textgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            (ZPC_BUTTON_HEIGHT + fonth) >> 1,
                            wininfo->str, len);
            } else {
                XDrawString(app->display, wininfo->id, wininfo->textgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            (ZPC_BUTTON_HEIGHT + fonth) >> 1,
                            wininfo->str, len);
            }
        }
    }
    XSync(app->display, False);

    return;
}

#if (HOVERBUTTONS)
void
buttonenter(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;

    if (!event->xexpose.count) {
        if (buttonpmaps[BUTTONHOVER]) {
            XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                       buttonpmaps[BUTTONHOVER]);
        }
        XClearWindow(app->display, wininfo->id);
        XSync(app->display, False);
    }

    return;
}

void
buttonleave(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;

    if (!event->xexpose.count) {
        XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                   buttonpmaps[BUTTONNORMAL]);
        XClearWindow(app->display, wininfo->id);
        XSync(app->display, False);
    }

    return;
}
#endif

void
zpcdel(void)
{
    struct zpcstkitem *item = zpcinputitem;

    if (item->str != item->scur) {
        *--item->scur = '\0';
        x11drawdisp();
    }

    return;
}

void
zpccls(void)
{
    struct zpcstkitem *item = zpcinputitem;

    item->scur = item->str;
    *item->str = '\0';

    return;
}

void
zpcclrstk(void)
{
    ;
}

void
zpcevaltop(void)
{
    struct zpcstkitem *item = zpcinputitem;

    if (item->scur == item->str) {
        zpcregstk[0] = zpcparse(zpcregstk[0]);
        zpcregstk[0] = zpceval(zpcregstk[0]);
        x11drawdisp();
    }

    return;
}

void
zpcenter(void)
{
    stkenterinput();
    x11drawdisp();

    return;
}

void
zpcquit(void)
{
    exit(0);
}

void
clientmessage(void *arg, XEvent *event)
{
    if (event->xany.window == mainwin && event->xclient.data.l[0] == wmdelete) {

        exit(0);
    }
}

void
buttonpress(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;
    int                 evbut = toevbutton(event->xbutton.button);
    struct zpctoken    *token;
    struct zpcstkitem  *item = zpcinputitem;
    zpccop_t           *func;
    zpcfop_t           *fltfunc;
    zpcaction_t        *action;
    struct zpctoken    *src;
    struct zpctoken    *dest;
    int64_t             res;
//    int64_t             src;
//    int64_t             dest;
//    int64_t             res64;
#if 0
    float               fsrc = NULL;
    float               fdest = NULL;
    float               fres;
#endif
#if 0
    double              dsrc;
    double              ddest;
    double              dres;
#endif
    long                type = 0;

    if (isaction(wininfo->parm)) {
        action = actiontab[toaction(wininfo->parm)];
        if (action) {
            action();
        }

        return;
    } else if (!wininfo->parm || item->scur != item->str) {
        if (buttonstrtab[wininfo->row][wininfo->col]) {
            stkqueueinput(buttonstrtab[wininfo->row][wininfo->col]);
            x11drawdisp();
        }

        return;
    } else {
        if (item->scur == item->str) {
            if (zpcregstk[0]) {
                token = zpcregstk[0];
                if (token) {
                    zpcfreequeue(token->next);
                    token->next = NULL;
                    if (token->type == ZPCINT64 || token->type == ZPCUINT64) {
                        type = token->type;
                        src = token;
                    } else {
                        fprintf(stderr, "EEEE\n");
                        
                        return;
                    }
                }
            } else {
                fprintf(stderr, "EEEE\n");
                
                return;
            }
            if (wininfo->parm == 2) {
                token = zpcregstk[1];
                if (token) {
                    zpcfreequeue(token->next);
                    token->next = NULL;
                    type = token->type;
                    dest = token;
                }
            }
        }
    }
#if 0
    XSetWindowBackgroundPixmap(app->display, wininfo->id,
                               buttonpmaps[BUTTONCLICKED]);
    XClearWindow(app->display, wininfo->id);
    XSync(app->display, False);
#endif
    if (evbut < NBUTTON) {
        token = calloc(1, sizeof(struct zpctoken));
        token->str = calloc(1, TOKENSTRLEN);
#if 0
        if  (!type) {
            type = ZPCINT64;
        }
#endif
        if (type == ZPCINT64 || type == ZPCUINT64) {
            func = wininfo->clickfunc[evbut];
            if (func) {
                if (type == ZPCINT64 || type == ZPCUINT64) {
                    res = func(src, dest);
                    token->type = type;
                    token->data.ui64.i64 = res;
                    switch (token->radix) {
                        case 2:
                            zpcconvbinuint64(res, token->str, TOKENSTRLEN);

                            break;
                        case 8:
                            sprintf(token->str, "%llo", res);

                            break;
                        case 10:
                            if (type == ZPCINT64) {
                                sprintf(token->str, "%lld", (int64_t)res);
                            } else {
                                sprintf(token->str, "%llu", res);
                            }

                            break;
                        case 16:
                        default:
                            sprintf(token->str, "0x%llx", res);

                            break;
                    }
                }
            }
#if 0
        } else {
            fltfunc = wininfo->clickfuncdbl[evbut];
            if (fltfunc) {
                dres = fltfunc(dsrc, ddest);
                token->type = ZPCDOUBLE;
                token->data.f64 = dres;
                sprintf(token->str, "%e", token->data.f64);
            }
#endif
        }
        if (func) {
            if (wininfo->parm >= 1) {
                struct zpctoken *token1 = zpcregstk[0];
                struct zpctoken *token2;

                while (token1) {
                    token2 = token1->next;
                    free(token1);
                    token1 = token2;
                }
                zpcregstk[0] = NULL;
                if (wininfo->parm == 2) {
                    token1 = zpcregstk[1];
                    while (token1) {
                        token2 = token1->next;
                        free(token1);
                        token1 = token2;
                    }
                    zpcregstk[1] = NULL;
                }
            }
            if (wininfo->parm == 2) {
                memmove(&zpcregstk[0], &zpcregstk[1], (NREGSTK - 1) * sizeof(struct zpctoken *));
                zpcregstk[NREGSTK - 1] = NULL;
            }
            zpcregstk[0] = token;
        }
    }
    x11drawdisp();

    return;
}

#if 0
void
buttonrelease(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;
    int            evbut = toevbutton(event->xbutton.button);
    copfunc_t     *func;
    int64_t       *src = &zpcregstktab[1]->data.ui64.i64;
    int64_t       *dest = &zpcregstktab[0]->data.ui64.i64;

    XSetWindowBackgroundPixmap(app->display, wininfo->id,
                               buttonpmaps[BUTTONNORMAL]);
    XClearWindow(app->display, wininfo->id);
    XSync(app->display, False);
}
#endif

void
x11init(void)
{
    Window             win;
    Window             dispwin;
    Window             regwin;
    Window             stkwin;
    int                row;
    int                col;
    struct x11wininfo *wininfo;

    app = x11initapp(NULL);
    font = x11loadfont("fixed");
    if (!font) {
        fprintf(stderr, "failed to load font fixed\n");

        exit(1);
    }
#if (ZPCIMLIB2)
    mainwin = x11initwin(app, 0,
                         0, 0,
                         ZPC_WINDOW_WIDTH,
                         ZPC_WINDOW_HEIGHT + NSTKREG * (fonth + 8) + NSTKREG + ((fonth + 8) << 1) + 1,
                         ZPCREVERSE);
#else
    mainwin = x11initwin(app, 0,
                         0, 0,
                         ZPC_WINDOW_WIDTH + ZPC_NCOLUMN - 1,
                         ZPC_WINDOW_HEIGHT + NSTKREG * (fonth + 8) + NSTKREG + ((fonth + 8) << 1) + 1,
                         ZPCREVERSE);
#endif
    if (mainwin) {
        app->win = mainwin;
        wininfo = calloc(1, sizeof(struct x11wininfo));
        wininfo->id = mainwin;
        wininfo->evfunc[ClientMessage] = clientmessage;
        x11addwininfo(wininfo);
    } else {
        fprintf(stderr, "failed to create application window\n");

        exit(1);
    }
    x11initwm();
    x11inittitle();
    x11initgcs();
#if (ZPCIMLIB2)
    imlib2init(app);
    x11initpmaps();
#endif
    for (row = 0 ; row < ZPC_NROW ; row++) {
        for (col = 0 ; col < ZPC_NCOLUMN ; col++) {
#if (ZPCIMLIB2)
            win = x11initwin(app,
                             mainwin,
                             col * ZPC_BUTTON_WIDTH,
                             row * ZPC_BUTTON_HEIGHT,
                             ZPC_BUTTON_WIDTH,
                             ZPC_BUTTON_HEIGHT,
                             ZPCREVERSE);
#else
            win = x11initwin(app,
                             mainwin,
                             col * ZPC_BUTTON_WIDTH + col,
                             row * ZPC_BUTTON_HEIGHT + row,
                             ZPC_BUTTON_WIDTH,
                             ZPC_BUTTON_HEIGHT,
                             ZPCREVERSE);
#endif
            if (win) {
                wininfo = calloc(1, sizeof(struct x11wininfo));
                wininfo->id = win;
                wininfo->str = buttonlabeltab[row][col];
                wininfo->topstr1 = buttontop1tab[row][col];
                wininfo->topstr2 = buttontop2tab[row][col];
                wininfo->type = buttontypetab[row][col];
                if (wininfo->str) {
                    if (wininfo->type == ZPCBUTTONASM) {
                        wininfo->textgc = asmtextgc;
                    } else if (wininfo->type == ZPCBUTTONOPER) {
                        wininfo->textgc = optextgc;
                    } else if (wininfo->type == ZPCBUTTONDIGIT) {
                        wininfo->textgc = numtextgc;
                    } else if (wininfo->type == ZPCBUTTONUTIL) {
                        wininfo->textgc = utiltextgc;
                    } else {
                        wininfo->textgc = textgc;
                    }
                }
                wininfo->parm = parmtab[row][col];
                wininfo->row = row;
                wininfo->col = col;
                wininfo->evfunc[ClientMessage] = clientmessage;
#if (HOVERBUTTONS)
                wininfo->evfunc[EnterNotify] = buttonenter;
                wininfo->evfunc[LeaveNotify] = buttonleave;
#endif
                wininfo->evfunc[ButtonPress] = buttonpress;
                wininfo->evfunc[Expose] = buttonexpose;
                buttonwintab[row][col] = win;
                wininfo->clickfunc[0] = buttonopertab[row][col];
//                wininfo->clickfuncdbl[0] = buttonopertabdbl[row][col];
                x11addwininfo(wininfo);
                XSelectInput(app->display, win,
#if (HOVERBUTTONS)
                             EnterWindowMask | LeaveWindowMask |
#endif
                             ExposureMask | ButtonPressMask);
                XMapRaised(app->display, win);
            } else {
                fprintf(stderr, "failed to create window\n");

                exit(1);
            }
        }
    }
#if (ZPCIMLIB2)
    dispwin = x11initwin(app,
                        mainwin,
                        0,
                        ZPC_WINDOW_HEIGHT,
                        ZPC_WINDOW_WIDTH,
                        NSTKREG * (fonth + 8) + NSTKREG + ((fonth + 8) << 2) + 1,
                        !ZPCREVERSE);
#else
    dispwin = x11initwin(app,
                        mainwin,
                        0,
                        ZPC_WINDOW_HEIGHT + ZPC_NROW,
                        ZPC_WINDOW_WIDTH,
                        NSTKREG * (fonth + 8) + NSTKREG + ((fonth + 8) << 2) + 1,
                        !ZPCREVERSE);
#endif
    XMapRaised(app->display, dispwin);
    for (row = 0 ; row < NSTKREG ; row++) {
        win = x11initwin(app,
                         dispwin,
                         0,
                         row * (fonth + 8) + row + 1,
                         ZPC_WINDOW_WIDTH,
                         fonth + 8,
                         ZPCREVERSE);
        if (win) {
            XMapRaised(app->display, win);
        }
        regwin = x11initwin(app,
                            win,
                            0,
                            0,
                            ZPC_SMALL_BUTTON_WIDTH,
                            ZPC_SMALL_BUTTON_HEIGHT,
                            ZPCREVERSE);
        if (regwin) {
            wininfo = calloc(1, sizeof(struct x11wininfo));
            wininfo->id = regwin;
            wininfo->num = NSTKREG - row - 1;
            wininfo->evfunc[Expose] = labelexpose;
            zpcregwintab[NSTKREG - row - 1] = regwin;
            x11addwininfo(wininfo);
            XSelectInput(app->display, regwin,
                         ExposureMask);
            XMapRaised(app->display, regwin);
        }
        stkwin = x11initwin(app,
                            win,
                            ZPC_SMALL_BUTTON_WIDTH,
                            0,
                            ZPC_WINDOW_WIDTH - ZPC_SMALL_BUTTON_WIDTH,
                            ZPC_SMALL_BUTTON_HEIGHT,
                            ZPCREVERSE);
        if (stkwin) {
            wininfo = calloc(1, sizeof(struct x11wininfo));
            wininfo->id = stkwin;
            wininfo->num = NSTKREG - row - 1;
            wininfo->evfunc[Expose] = displayexpose;
            stkwintab[NSTKREG - row - 1] = stkwin;
            x11addwininfo(wininfo);
            XSelectInput(app->display, stkwin,
                         ExposureMask);
            XMapRaised(app->display, stkwin);
        }
    }
    inputwin = x11initwin(app,
                          dispwin,
                          0,
                          NSTKREG * (fonth + 8) + NSTKREG + 1,
                          ZPC_WINDOW_WIDTH,
                          (fonth + 8) << 2,
                          ZPCREVERSE);
    if (inputwin) {
        wininfo = calloc(1, sizeof(struct x11wininfo));
        wininfo->id = win;
        wininfo->evfunc[Expose] = displayexpose;
        x11addwininfo(wininfo);
        XSelectInput(app->display, inputwin,
                     ExposureMask);
        XMapRaised(app->display, inputwin);
    }
    XMapSubwindows(app->display, mainwin);
    XMapSubwindows(app->display, dispwin);
    XMapRaised(app->display, mainwin);
}

void
x11nextevent(void)
{
    XEvent             event;
    struct x11wininfo *wininfo;
    x11evfunc_t       *func;

    XNextEvent(app->display, &event);
    wininfo = x11findwininfo(event.xany.window);
    if (wininfo) {
//        fprintf(stderr, "WIN: %ld (%ld), EVENT: %d\n",
//                event.xany.window, mainwin, event.type);
        func = wininfo->evfunc[event.type];
        if (func) {
            func(wininfo, &event);
        }
    }

    return;
}

#endif /* ZPCX11 */

