#include <stdlib.h>
#include <stdio.h>
#include <ui/ui.h>
#include <ui/sys/xorg.h>

char *
uigetdisp_x11(int argc, char *argv[])
{
    char *dispname = NULL;
    int   ndx;

    for (ndx = 1 ; ndx < argc ; ndx++) {
        if (!strncmp(argv[ndx], "-D", 2)
            || !strncmp(argv[ndx], "--display", 9)) {
            ndx++;
            if (ndx < argc) {
                dispname = argv[ndx];
            }
        }
    }

    return dispname;
}

void
uiinit_xorg(struct ui *ui, char argc, char *argv[])
{
    Display          *disp;
    char             *dispname = uigetdisp_x11(argc, argv);
    struct uienv_x11 *env = malloc(sizeof(struct uienv_x11));
    int               i;

    if (!env) {
        fprintf(stderr, "UI: failed to allocate environment\n");

        exit(1);
    }
    ui->env = env;
    fprintf(stderr, "UI: opening display %s\n", dispname);
    XInitThreads();
    disp = XOpenDisplay(dispname);
    if (!disp) {
        fprintf(stderr, "failed to open display %s\n", dispname);

        exit(1);
    }
    env->display = disp;
    i = DefaultScreen(disp);
    env->screen = i;
    env->depth = DefaultDepth(disp, i);
    env->visual = DefaultVisual(disp, i);
    env->colormap = DefaultColormap(disp, i);

    return;
}

struct uiwin *
uimkwin_x11(struct ui *ui, Window parent,
            int x, int y,
            int w, int h)
{
    Window                 id;
    struct uienv_x11      *env = ui->env;
    struct uiwin          *win = malloc(sizeof(struct uiwin));
    void                 **evtab;
    XSetWindowAttributes   atr = { 0 };

    if (win) {
        if (!parent) {
            parent = RootWindow(env->display, env->screen);
        }
        id = XCreateWindow(env->display,
                           parent,
                           x, y,
                           w, h,
                           0,
                           env->depth,
                           InputOutput,
                           env->visual,
                           0,
                           &atr);
        if (!id) {
            free(win);
        }
        win->id = id;
        evtab = calloc(UIX11NDEFEV, sizeof(void *));
        if (!evtab) {
            fprintf(stderr, "UI: failed to allocate function table\n");
            free(win);

            exit(1);
        }
        win->evtab = evtab;
    }

    return win;
}

#if (UITESTX11)

#include <stdio.h>

int
main(int argc, char *argv[])
{
    struct ui     ui;
    struct uiwin *win;

    uiinit_xorg(&ui, argc, argv);
    win = uimkwin_x11(&ui, UIX11ROOTWINID,
                      0, 0,
                      16, 16);
    fprintf(stderr, "%p\n", win);

    exit(0);
}

#endif /* UITESTX11 */

