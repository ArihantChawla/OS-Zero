#include <vt/conf.h>
#if (VTTEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <zero/trix.h>
#include <ui/ui.h>
#include <ui/text.h>
#include <vt/vt.h>
#include <vt/pty.h>
#include <vt/color.h>

int32_t  vtxtermcolortab[256] ALIGNED(PAGESIZE) = VT_XTERM_COLORMAP;
char    *vtkeystrtab[128] ALIGNED(CLSIZE);
int32_t  vtdefcolortab[16] ALIGNED(CLSIZE) = VT_DEFAULT_COLORMAP;

void
vtsetsigs(void)
{
//    signal(SIGHUP, termexitsig);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    signal(SIGSEGV, SIG_DFL);
    signal(SIGBUS, SIG_DFL);
    signal(SIGABRT, SIG_DFL);
    signal(SIGFPE, SIG_DFL);
    signal(SIGILL, SIG_DFL);
#if defined(SIGSYS)
    signal(SIGSYS, SIG_DFL);
#endif
    signal(SIGALRM, SIG_DFL);
    signal(SIGTSTP, SIG_DFL);
    signal(SIGTTIN, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGCHLD, SIG_DFL);

    return;
};

#if (TERMXORG)
void
vtinitconn(struct vt *vt)
{
    struct uienv_xorg *env = vt->ui.env;

    vt->connfd = ConnectionNumber(env->display);

    return;
}

long
vtinitscr(struct vt *vt, long n)
{
    return 0;
}

long
vtinitwin(struct vt *vt)
{
    return 0;
}

void
vtmapwin(struct vt *vt)
{
    return;
}

void
vtsyncwin(struct vt *vt)
{
    struct uienv_xorg *env = vt->ui.env;

    XSync(env->display, False);
}

void
vtfreescr(struct vt *vt)
{
    return;
}
#endif

struct vt *
vtrun(struct vt *vt)
{
    char         *path;
    struct vtbuf *buf;
//    pid_t        pid;

    if (!vt) {
        vt = malloc(sizeof(struct vt));
    }
//    vt-> = vt;
    /* set signals up */
    vtsetsigs();
    /*
     * set input field separator for shell to TAB for old shells that don't
     * reset it. This is for security reasons to prevent users from setting
     * IFS to make system() execute a different program.
     */
    putenv("IFS= \t");
    /* export terminal type */
    putenv("TERM=vt100");
    /* allocate buffer */
    buf = calloc(1, sizeof(struct vtbuf));
    if (!buf) {
        vtfree(vt);
        fprintf(stderr, "out of memory\n");

        exit(1);
    }
    vt->buf = buf;
    /* allocate and read path */
    path = malloc(PATH_MAX);
    if (!path) {
        vtfree(vt);
        fprintf(stderr, "out of memory\n");

        exit(1);
    }
    if (!getcwd(path, PATH_MAX)) {
        vtfree(vt);
        free(path);
        fprintf(stderr, "out of memory\n");

        exit(1);
    }
    vt->path = path;
    /* initialise desktop connection */
    vtinitconn(vt);
    /* initialise terminal screens */
    if (!vtinitscr(vt, TERMNSCREEN)) {
        vtfree(vt);
        free(path);

        exit(1);
    }
    /* initialise terminal windows */
    if (!vtinitwin(vt)) {
        vtfree(vt);
        free(path);
        vtfreescr(vt);

        exit(1);
    }
    /* map terminal windows */
    vtmapwin(vt);
    /* synchronize desktop */
    vtsyncwin(vt);

    return vt;
}

void
vtfreecolors(struct vt *vt)
{
    void *tab;

    tab = vt->colormap.deftab;
    if (tab != vtdefcolortab) {
        free(tab);
    }
    tab = vt->colormap.xtermtab;
    if (tab != vtxtermcolortab) {
        free(tab);
    }

    return;
}

void
vtfreefonts(struct vt *vt)
{
    struct uiapi *api = vt->ui.api;

    if (api->freefont) {
        api->freefont(&vt->ui, &vt->font);
    }

    return;
}

void
vtfree(struct vt *vt)
{
    void *ptr;

    vtfreebuf(vt);
    vtfreepty(vt);
    ptr = vt->state.tabmap;
    if (ptr) {
        free(ptr);
        vt->state.tabmap = NULL;
    }
    uifreetextbuf(&vt->textbuf);
    uifreetextbuf(&vt->scrbuf);
    vtfreecolors(vt);
    vtfreefonts(vt);

    return;
}

long
vtinitcolors(struct vt *vt)
{
    void              *deftab;
    void              *xtermtab;

    deftab = uiinitcolors(&vt->ui, vtdefcolortab, 16);
    if (!deftab) {

        return 0;
    }
    xtermtab = uiinitcolors(&vt->ui, vtxtermcolortab, 256);
    if (!xtermtab) {
        free(deftab);

        return 0;
    }
    vt->colormap.deftab = deftab;
    vt->colormap.xtermtab = xtermtab;

    return 1;
}

long
vtinitfonts(struct vt *vt)
{

    if (!uiinitfont(&vt->ui, &vt->font, VTDEFFONT)) {

        return 0;
    }

    return 1;
}

#if 0
long
vtinitcolors(struct vt *vt)
{
    struct uienv_xorg *env = vt->ui.env;
    struct uiapi      *api = vt->ui.api;
    void              *deftab;
    void              *xtermtab;

    if (api->initcolors) {
        deftab = api->initcolors(env, vtdefcolortab, 16);
        if (!deftab) {
            
            return 0;
        }
        xtermtab = api->initcolors(env, vtxtermcolortab, 256);
        if (!xtermtab) {
            free(deftab);
            
            return 0;
        }
    } else {
        deftab = vtdefcolortab;
        xtermtab = vtxtermcolortab;
    }
    vt->colormap.deftab = deftab;
    vt->colormap.xtermtab = xtermtab;

    return 1;
}

long
vtinitfonts(struct vt *vt)
{
    struct uiapi *api = vt->ui.api;

    if (api->initfont) {
        if (!api->initfont(&vt->ui, &vt->font, VTDEFFONT)) {

            return 0;
        }
    }

    return 1;
}
#endif

struct vt *
vtinit(struct vt *vt, int argc, char *argv[])
{
    long  newvt = (vt) ? 0 : 1;
    void *ptr;

    if (!vt) {
        vt = calloc(1, sizeof(struct vt));
        if (!vt) {

            return NULL;
        }
    }
    vt->atr.fd = -1;
    if (!vtinitbuf(vt)
        || !vtinitpty(vt)) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    vt->state.mode = VTDEFMODE;
    vt->state.fgcolor = VTDEFFGCOLOR;
    vt->state.bgcolor = VTDEFBGCOLOR;
    vt->state.textatr = VTDEFTEXTATR;
    uisetsys(&vt->ui, UI_SYS_XORG);
    uiinit(&vt->ui, argc, argv);
    if (!vtinitcolors(vt)) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }
        
        return NULL;
    }
    if (!vtinitfonts(vt)) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }
        
        return NULL;
    }
    ptr = calloc(rounduppow2(vt->state.ncol, 32) >> 5, sizeof(uint32_t));
    if (!ptr) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }
        
        return NULL;
    }
    vt->state.tabmap = ptr;
    vtinitesc(vt);
    vt->state.scrolltop = 0;
    vt->state.scrollbottom = vt->state.ncol - 1;

    return vt;
}

