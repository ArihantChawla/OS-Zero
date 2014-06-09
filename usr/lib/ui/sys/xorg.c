#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ui/ui.h>
#include <ui/sys/xorg.h>

char *
uigetdisp_xorg(int argc, char *argv[])
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

void *
uiinitcolors_xorg(void *env, int32_t *tab, size_t n)
{
    struct uienv_xorg *xenv = env;
    unsigned long     *data;
    unsigned long      pixel;
    long               ndx;
    XColor             color;

    data = malloc(n * sizeof(unsigned long));
    if (!data) {

        return NULL;
    }
    color.flags = DoRed | DoGreen | DoBlue;
    for (ndx = 0 ; ndx < 16 ; ndx++) {
        pixel = tab[ndx];
        color.red = pixel >> 16;
        color.green = (pixel >> 8) & 0xff;
        color.blue = pixel & 0xff;
        if (!XAllocColor(xenv->display,
                         xenv->colormap,
                         &color)) {
            free(data);
            
            return NULL;
        }
        data[ndx] = color.pixel;
    }

    return data;
}

void
uiinit_xorg(struct ui *ui, int argc, char *argv[])
{
    Display           *disp;
    char              *dispname = uigetdisp_xorg(argc, argv);
    struct uienv_xorg *env = malloc(sizeof(struct uienv_xorg));
    struct uiapi_xorg *api;
    int                i;

    if (!env) {
        fprintf(stderr, "UI: failed to allocate environment\n");

        exit(1);
    }
    ui->env = env;
    api = malloc(sizeof(struct uienv_xorg));
    if (!api) {
        fprintf(stderr, "UI: failed to allocate API\n");

        exit(1);
    }
    ui->api = api;
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
    api->initcolors = uiinitcolors_xorg;

    return;
}

struct uiwin *
uimkwin_xorg(struct ui *ui, Window parent,
            int x, int y,
            int w, int h)
{
    Window                 id;
    struct uienv_xorg     *env = ui->env;
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
        evtab = calloc(UIXORGNDEFEV, sizeof(void *));
        if (!evtab) {
            fprintf(stderr, "UI: failed to allocate function table\n");
            free(win);

            exit(1);
        }
        win->evtab = evtab;
    }

    return win;
}

#if (UITESTXORG)

#include <stdio.h>

int
main(int argc, char *argv[])
{
    struct ui     ui;
    struct uiwin *win;

    uiinit_xorg(&ui, argc, argv);
    win = uimkwin_xorg(&ui, UIXORGROOTWINID,
                      0, 0,
                      16, 16);
    fprintf(stderr, "%p\n", win);

    exit(0);
}

#endif /* UITESTXORG */

