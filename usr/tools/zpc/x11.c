#if (ZPCX11)

/* TODO: add buttons '<', '>', and 'u' */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/keysymdef.h>
#include <X11/Xutil.h>
#include <zpc/conf.h>
#include <zpc/zpc.h>
#include <zpc/op.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include "x11.h"

#define HOVERBUTTONS 1

#define ZPC_TITLE "Zero Programmer's Calculator"

void stkenterinput(void);
void stkqueueinput(const char *str);

void zpcdel(void);
void zpccls(void);
void zpcclrstk(void);
void zpcevaltop(void);
void zpcenter(void);
void zpcquit(void);
void zpcprog(void);

extern uint8_t            zpcoperchartab[256];
extern struct zpctoken   *zpcregstk[];
extern struct zpcstkitem *zpcinputitem;
extern long               zpcradix;
#define NHASHITEM 1024
static struct x11wininfo *winhash[NHASHITEM] ALIGNED(PAGESIZE);
static x11keyhandler_t   *keypressfunctab[256];
static zpcop_t           *buttonopertab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, not64, shra64, inc64, NULL },
    { NULL, NULL, NULL, mod64, shr64, dec64, NULL },
    { NULL, NULL, NULL, div64, shl64, ror64, NULL },
    { NULL, NULL, NULL, mul64, xor64, rol64, NULL },
    { NULL, NULL, NULL, sub64, or64, NULL, NULL },
    { NULL, NULL, NULL, add64, and64, NULL, NULL }
};
#if 0
static zpcfop_t        *buttonopertabdbl[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fdiv64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, mul64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fsub64, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, fadd64, NULL, NULL, NULL}
};
#endif
static Window             buttonwintab[ZPC_NROW][ZPC_NCOLUMN];
static uint8_t            buttontypetab[ZPC_NROW][ZPC_NCOLUMN] =
{
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONDEBUG },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONDIGIT }, 
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER },
    { ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONDIGIT, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER, ZPCBUTTONOPER }
};
static const char        *buttonstrtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { "d", "e", "f", "~", ">>>", "++", "PC" },
    { "a", "b", "c", "%", ">>", "--", "0x" },
    { "7", "8", "9", "/", "<<", "..>", "(" },
    { "4", "5", "6", "*", "^", "<..", ")" },
    { "1", "2", "3", "-", "|", "=", " " },
    { "0", NULL, ",", "+", "&", NULL, "ENTER" }
};
static const char        *buttonlabeltab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { "d", "e", "f", "~", ">>>", "++", "PC" },
    { "a", "b", "c", "%", ">>", "--", "0x" },
    { "7", "8", "9", "/", "<<", "..>", "(" },
    { "4", "5", "6", "*", "^", "<..", ")" },
    { "1", "2", "3", "-", "|", "=", "SPACE" },
    { "0", ".", ",", "+", "&", "EVAL", "ENTER" }
};
static const char        *buttontop1tab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, "not", "shra", "inc", NULL },
    { NULL, NULL, NULL, "mod", "shr", "dec", NULL },
    { NULL, NULL, NULL, "div", "shl", "ror", NULL },
    { NULL, NULL, NULL, "mul", "xor", "rol", NULL },
    { NULL, NULL, NULL, "sub", "or", NULL, NULL },
    { NULL, NULL, NULL, "add", "and", NULL, NULL }
};
static const char        *buttontop2tab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, NULL, NULL, NULL, NULL },
    { NULL, NULL, NULL, "fdiv", NULL, NULL, NULL },
    { NULL, NULL, NULL, "fmul", NULL, NULL, NULL },
    { NULL, NULL, NULL, "fsub", NULL, NULL, NULL },
    { NULL, NULL, NULL, "fadd", NULL, NULL, NULL }
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
#define RUN   0xf5
#define STEP  0xf4
#define BREAK 0xf3
#define LOAD  0xf2
#define SAVE  0xf1
#define PROG  0xf0
#define FIRST 0xf0
#if 0
#define EQU   0xf0
#define VEC   0xef
#define MAT   0xee
#define PLOT  0xed
#define RAD   0xec
#define UNIT  0xeb
#define NEG   0xea
#endif
#define isaction(i)                                                     \
    ((i) >= FIRST && (i) <= 0xff)
#define toaction(i)                                                     \
    ((i) - FIRST)
static const uint8_t      parmtab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { 0, 0, 0, 1, 2, 1, PROG },
    { 0, 0, 0, 2, 2, 1, 0 },
    { 0, 0, 0, 2, 2, 2, 0 },
    { 0, 0, 0, 2, 2, 2, 0 },
    { 0, 0, 0, 2, 2, 0, 0 },
    { 0, 0, 0, 2, 2, EVAL, ENTER }
};
#if 0
static const uint8_t      isflttab[ZPC_NROW][ZPC_NCOLUMN]
= {
    { 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 1, 0, 0 },
    { 0, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0, 0, 0 }
};
#endif
static zpcaction_t *actiontab[0xff - FIRST + 1] =
{
    zpcprog,    // PROG
    NULL,       // SAVE
    NULL,       // LOAD
    NULL,       // BREAK
    NULL,       // STEP
    NULL,       // RUN
    zpccls,     // CLS
    zpcdel,     // DEL
    NULL,       // DROP
    NULL,       // DUP
    NULL,       // POP
    NULL,       // COPY
    zpcclrstk,  // CLR
    zpcevaltop, // EVAL
    zpcenter,   // ENTER
    zpcquit     // QUIT
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
static uint8_t zpcregstrlentab[NREGSTK]
= {
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    3,
    4,
    4,
    4,
    4,
    4,
    4
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
GC                    debugtextgc;
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
            len = token->len;
            if (!len) {
                token->len = len = strlen(token->str);
            }
            XDrawString(app->display, win, textgc,
                        x,
                        (fonth >> 1) + 8,
                        token->str, len);
            x += len * fontw;
            token = token->next;
        }
        str = zpcregstrtab[i];
        len = zpcregstrlentab[i];
        win = zpcregwintab[i];
        x = 8;
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
        fprintf(stderr, "failed to allocate color 'orange'\n");

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
        fprintf(stderr, "failed to allocate color 'green'\n");

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
        fprintf(stderr, "failed to allocate color 'light blue'\n");

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
    if (!XParseColor(app->display,
                     app->colormap,
                     "yellow",
                     &color)) {
        fprintf(stderr, "failed to parse color 'yellow'\n");

        exit(1);
    }
    if (!XAllocColor(app->display,
                     app->colormap,
                     &color)) {
        fprintf(stderr, "failed to allocate color 'yellow'\n");

        exit(1);
    }
    gcval.foreground = color.pixel;
    debugtextgc = XCreateGC(app->display, app->win,
                            GCForeground | GCFont | GCGraphicsExposures,
                            &gcval);
    if (!debugtextgc) {
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
    smallbuttonpmaps[BUTTONNORMAL] = imlib2loadimage(app, "buttonsmall.png",
                                                     ZPC_SMALL_BUTTON_WIDTH,
                                                     ZPC_SMALL_BUTTON_HEIGHT);
#if (SMALLBUTTONS)
    buttonpmaps[BUTTONNORMAL] = imlib2loadimage(app, "button.png",
                                                ZPC_BUTTON_WIDTH,
                                                ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONHOVER] = imlib2loadimage(app, "buttonhilite.png",
                                               ZPC_BUTTON_WIDTH,
                                               ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONCLICKED] = imlib2loadimage(app, "buttonpress.png",
                                                 ZPC_BUTTON_WIDTH,
                                                 ZPC_BUTTON_HEIGHT);
#else
    buttonpmaps[BUTTONNORMAL] = imlib2loadimage(app, "button_small.png",
                                                ZPC_BUTTON_WIDTH,
                                                ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONHOVER] = imlib2loadimage(app, "buttonhilite_small.png",
                                               ZPC_BUTTON_WIDTH,
                                               ZPC_BUTTON_HEIGHT);
    buttonpmaps[BUTTONCLICKED] = imlib2loadimage(app, "buttonpress_small.png",
                                                 ZPC_BUTTON_WIDTH,
                                                 ZPC_BUTTON_HEIGHT);
#endif
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
zpcdrawlabel(struct x11wininfo *wininfo)
{
    size_t len;

    if (wininfo->str) {
#if (!SMALLBUTTONS)
        if (wininfo->topstr2) {
            len = strlen(wininfo->topstr2);
            XDrawString(app->display, wininfo->id, asmtextgc,
                        ZPC_BUTTON_WIDTH - len * fontw - 4,
                        fonth + 4,
                        wininfo->topstr2, len);
            if (wininfo->topstr1) {
                len = strlen(wininfo->topstr1);
                XDrawString(app->display, wininfo->id, asmtextgc,
                            4,
                            fonth + 4,
                            wininfo->topstr1, len);
                }
        } else
#endif
        if (wininfo->topstr1) {
            len = strlen(wininfo->topstr1);
            if (len) {
#if (SMALLBUTTONS)
                XDrawString(app->display, wininfo->id, asmtextgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            fonth,
                            wininfo->topstr1, len);
#else
                XDrawString(app->display, wininfo->id, asmtextgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            fonth + 4,
                            wininfo->topstr1, len);
#endif
            }
        }
        len = strlen(wininfo->str);
        if (len) {
            if (wininfo->topstr1) {
#if (SMALLBUTTONS)
                XDrawString(app->display, wininfo->id, wininfo->textgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            ((ZPC_BUTTON_HEIGHT + fonth) >> 1) + 4,
                            wininfo->str, len);
#else
                XDrawString(app->display, wininfo->id, wininfo->textgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            (ZPC_BUTTON_HEIGHT + fonth) >> 1,
                            wininfo->str, len);
#endif
            } else {
                XDrawString(app->display, wininfo->id, wininfo->textgc,
                            (ZPC_BUTTON_WIDTH - len * fontw) >> 1,
                            (ZPC_BUTTON_HEIGHT + fonth) >> 1,
                            wininfo->str, len);
            }
        }
    }

    return;
}

void
calcenter(void *arg, XEvent *event)
{
    XSetInputFocus(app->display, mainwin, RevertToPointerRoot, CurrentTime);

    return;
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
            len = token->len;
            if (!len) {
                token->len = len = strlen(token->str);
            }
            XDrawString(app->display, wininfo->id, textgc,
                        x,
                        (fonth >> 1) + 8,
                        token->str, len);
            x += len * fontw;
            token = token->next;
        }
    }

    return;
}

void
labelexpose(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;
    char              *str = zpcregstrtab[wininfo->num];
    int                x;
    int                len;

    if (!event->xexpose.count) {
        x = 8;
        XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                   smallbuttonpmaps[BUTTONNORMAL]);
        XClearWindow(app->display, wininfo->id);
        len = zpcregstrlentab[wininfo->num];
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

    if (!event->xexpose.count) {
        if (buttonpmaps[wininfo->state]) {
            XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                       buttonpmaps[wininfo->state]);
        }
        XClearWindow(app->display, wininfo->id);
        zpcdrawlabel(wininfo);
    } else {
        XClearWindow(app->display, wininfo->id);
    }
    XSync(app->display, False);

    return;
}

#if (HOVERBUTTONS)
void
buttonenter(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;

    if (buttonpmaps[BUTTONHOVER]) {
        wininfo->state = BUTTONHOVER;
        XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                   buttonpmaps[BUTTONHOVER]);
    }
    XClearWindow(app->display, wininfo->id);
    zpcdrawlabel(wininfo);
    XSync(app->display, False);

    return;
}

void
buttonleave(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;

    if (buttonpmaps[BUTTONNORMAL]) {
        wininfo->state = BUTTONNORMAL;
        XSetWindowBackgroundPixmap(app->display, wininfo->id,
                                   buttonpmaps[BUTTONNORMAL]);
    }
    XClearWindow(app->display, wininfo->id);
    zpcdrawlabel(wininfo);
    XSync(app->display, False);

    return;
}
#endif

void
keypress(void *arg, XEvent *event)
{
    struct zpcstkitem *item = zpcinputitem;
    unsigned long      keysym = event->xkey.keycode;
    char               ch;
    x11keyhandler_t   *func;
    XComposeStatus     comp;
    char               str[16];

    XLookupString(&(event->xkey), str, 16, &keysym, &comp);
    if (keysym == XK_Return || keysym == XK_KP_Enter) {
        stkenterinput();
    } else if (keysym <= 0xff) {
        ch = keysym;
        if (item->str == item->scur && zpcisoperchar(ch)) {
            func = keypressfunctab[(int)ch];
            if (func) {
                ;
            }
        } else {
            str[0] = ch;
            str[1] = '\0';
            stkqueueinput(str);
        }
    }
    x11drawdisp();

    return;
}

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
zpcprog(void)
{
    ;
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
    zpcop_t            *func = NULL;
//    zpcfop_t           *fltfunc;
    zpcaction_t        *action;
    struct zpctoken    *src = NULL;
    struct zpctoken    *dest = NULL;
    struct zpctoken    *dtok;
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
    long                radix;

    XSetWindowBackgroundPixmap(app->display, wininfo->id,
                               buttonpmaps[BUTTONCLICKED]);
    XClearWindow(app->display, wininfo->id);
    zpcdrawlabel(wininfo);
    XSync(app->display, False);
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
    } else if (item->scur == item->str) {
        if (zpcregstk[0]) {
            token = zpcregstk[0];
            if (token) {
                if (token->type == ZPCINT64 || token->type == ZPCUINT64) {
                    type = token->type;
                    src = token;
                } else {
                    fprintf(stderr, "EEEE\n");
                    
                    return;
                }
                zpcfreequeue(token->next);
                token->next = NULL;
            }
        } else {
            fprintf(stderr, "empty stack\n");
            
            return;
        }
        if (wininfo->parm == 2) {
            token = zpcregstk[1];
            if (token) {
                zpcfreequeue(token->next);
                token->next = NULL;
                type = token->type;
                dest = token;
            } else {
                fprintf(stderr, "missing argument #2\n");
                
                return;
            }
        }
    } else {
        
        return;
    }
    if (evbut < NBUTTON) {
        token = calloc(1, sizeof(struct zpctoken));
        token->str = calloc(1, TOKENSTRLEN);
        token->slen = TOKENSTRLEN;
#if 0
        if  (!type) {
            type = ZPCINT64;
        }
#endif
        if (type == ZPCINT64 || type == ZPCUINT64) {
            func = wininfo->clickfunc[evbut];
            if (func) {
                token->type = type;
                res = func(src, dest);
                if (dest) {
                    dtok = dest;
                } else {
                    dtok = src;
                }
#if (SMARTRADIX)
                if (src->radix == 16 || dest->radix == 16) {
                    token->radix = 16;
                } else if (src->radix == 8 || dest->radix == 8) {
                    token->radix = 8;
                } else if (src->radix == 2 || dest->radix == 2) {
                    token->radix = 2;
                } else {
//                    token->radix = 10;
                    token->radix = zpcradix;
                }
#else
                if (dtok->radix == 16) {
                    token->radix = 16;
                } else if (dtok->radix == 8) {
                    token->radix = 8;
                } else if (dtok->radix == 2) {
                    token->radix = 2;
                } else {
//                    token->radix = 10;
                    token->radix = zpcradix;
                }
#endif
                token->radix = zpcradix;
#if (SMARTTYPES)
                token->type = dtok->type;
                token->flags = dtok->flags;
#endif
                token->data.ui64.i64 = res;
//                token->sign = dtok->sign;
                radix = token->radix;
                if (!radix) {
                    token->radix = radix = zpcradix;
                }
                zpcprintstr64(token, res, radix);
            }
#if 0
        } else {
            fltfunc = wininfo->clickfuncdbl[evbut];
            if (fltfunc) {
                dres = fltfunc(dsrc, dtoken);
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
                memmove(&zpcregstk[1], &zpcregstk[2],
                        (NREGSTK - 2) * sizeof(struct zpctoken *));
                zpcregstk[NREGSTK - 1] = NULL;
            }
            zpcregstk[0] = token;
        }
    }
    x11drawdisp();

    return;
}

void
buttonrelease(void *arg, XEvent *event)
{
    struct x11wininfo *wininfo = arg;

    XSetWindowBackgroundPixmap(app->display, wininfo->id,
                               buttonpmaps[wininfo->state]);
    XClearWindow(app->display, wininfo->id);
    zpcdrawlabel(wininfo);
    XSync(app->display, False);
}

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
    font = x11loadfont(ZPC_FONT_NAME);
    if (!font) {
        font = x11loadfont("fixed");
    }
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
        wininfo->evfunc[EnterNotify] = calcenter;
        wininfo->evfunc[KeyPress] = keypress;
        XSelectInput(app->display, mainwin, EnterWindowMask | KeyPressMask);
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
                    } else if (wininfo->type == ZPCBUTTONDEBUG) {
                        wininfo->textgc = debugtextgc;
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
                wininfo->evfunc[ButtonRelease] = buttonrelease;
                wininfo->evfunc[Expose] = buttonexpose;
                buttonwintab[row][col] = win;
                wininfo->clickfunc[0] = buttonopertab[row][col];
//                wininfo->clickfuncdbl[0] = buttonopertabdbl[row][col];
                x11addwininfo(wininfo);
                XSelectInput(app->display, win,
#if (HOVERBUTTONS)
                             EnterWindowMask | LeaveWindowMask |
#endif
                             ExposureMask | ButtonPressMask
                             | ButtonReleaseMask);
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

