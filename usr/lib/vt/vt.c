#include <vt/conf.h>
#if (VTTEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <limits.h>
#include <zero/cdecl.h>
#include <zero/param.h>
#include <ui/ui.h>
#include <vt/vt.h>
#include <vt/pty.h>
#include <vt/color.h>
#include <vt/xterm.h>

int32_t vtxtermcolortab[256] ALIGNED(PAGESIZE) = VT_XTERM_COLORMAP;

#define vtdefcolor(i)     (vtdefcolortab[(c)])
#define vtfgtodefcolor(i) ((c) - 30)
#define vtbgtodefcolor(i) ((c) - 40)
int32_t vtdefcolortab[16] ALIGNED(CLSIZE) = {
    VT_BLACK_COLOR,
    VT_RED_COLOR,
    VT_GREEN_COLOR,
    VT_YELLOW_COLOR,
    VT_BLUE_COLOR,
    VT_MAGENTA_COLOR,
    VT_CYAN_COLOR,
    VT_WHITE_COLOR,
    VT_BRIGHT_BLACK_COLOR,
    VT_BRIGHT_RED_COLOR,
    VT_BRIGHT_GREEN_COLOR,
    VT_BRIGHT_YELLOW_COLOR,
    VT_BRIGHT_BLUE_COLOR,
    VT_BRIGHT_MAGENTA_COLOR,
    VT_BRIGHT_CYAN_COLOR,
    VT_BRIGHT_WHITE_COLOR
};

void
vtfree(struct vt *vt)
{
    if (vt->devbuf.in.base) {
        free(vt->devbuf.in.base);
        vt->devbuf.in.base = NULL;
    }
    if (vt->devbuf.out.base) {
        free(vt->devbuf.out.base);
        vt->devbuf.out.base = NULL;
    }
    if (vt->atr.masterpath) {
        free(vt->atr.masterpath);
        vt->atr.masterpath = NULL;
    }
    if (vt->atr.slavepath) {
        free(vt->atr.slavepath);
        vt->atr.masterpath = NULL;
    }

    return;
}

void
vtfreetextbuf(struct vttextbuf *buf)
{
    int32_t       **data = buf->data;
    struct vtrend **rend = buf->rend;
    long            nrow = buf->nrow;
    long            n;

    if (data) {
        for (n = 0 ; n < nrow ; n++) {
            if (data[n]) {
                free(data[n]);
            }
        }
        free(data);
    }
    if (rend) {
        for (n = 0 ; n < nrow ; n++) {
            if (rend[n]) {
                free(rend[n]);
            }
        }
        free(rend);
    }
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

long
vtinittextbuf(struct vttextbuf *buf, long nrow, long ncol)
{
    long            ndx;
    int32_t       **data;
    struct vtrend **rend;
    int32_t        *dptr;
    struct vtrend  *rptr;

    data = malloc(nrow * sizeof(int32_t *));
    if (!data) {

        return 0;
    }
    rend = malloc(nrow * sizeof(struct vtrend *));
    if (!rend) {
        vtfreetextbuf(buf);
        
        return 0;
    }
    for (ndx = 0 ; ndx < nrow ; ndx++) {
        dptr = calloc(ncol, sizeof(int32_t));
        if (!dptr) {
            vtfreetextbuf(buf);

            return 0;
        }
        data[ndx] = dptr;
        rptr = calloc(ncol, sizeof(struct vtrend));
        if (!rptr) {
            vtfreetextbuf(buf);

            return 0;
        }
        rend[ndx] = rptr;
    }
    buf->data = data;
    buf->rend = rend;

    return 1;
}

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

struct vt *
vtinit(struct vt *vt, int argc, char *argv[])
{
    long  newvt = (vt) ? 0 : 1;

    if (!vt) {
        vt = malloc(sizeof(struct vt));
        if (!vt) {

            return vt;
        }
    }
    if (!ringinit(&vt->devbuf.in, NULL, VTBUFSIZE / sizeof(RING_ITEM))
        || !ringinit(&vt->devbuf.in, NULL, VTBUFSIZE / sizeof(RING_ITEM))
        || !(vt->atr.masterpath = malloc(PATH_MAX))
        || !(vt->atr.slavepath = malloc(PATH_MAX))) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }
        
        return NULL;
    }
    vt->atr.fd = vtopenpty(&vt->atr.masterpath, &vt->atr.slavepath);
    if (vt->atr.fd < 0) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    vt->state.mode = VTDEFMODE;
    vt->state.flags = 0;
    vt->state.fgcolor = VTDEFFGCOLOR;
    vt->state.bgcolor = VTDEFBGCOLOR;
    vt->state.textatr = VTDEFTEXTATR;
    if (!vtinittextbuf(&vt->textbuf, vt->textbuf.nrow, vt->state.ncol)) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    if (!vtinittextbuf(&vt->scrbuf, vt->state.nrow, vt->state.ncol)) {
        vtfree(vt);
        vtfreetextbuf(&vt->textbuf);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    uisetsys(&vt->ui, UI_SYS_XORG);
    uiinit(&vt->ui, argc, argv);
    if (!vtinitcolors(vt)) {
        vtfree(vt);
        vtfreetextbuf(&vt->textbuf);
        vtfreetextbuf(&vt->scrbuf);
        if (newvt) {
            free(vt);
        }
        
        return NULL;
    }
    if (!vtinitfonts(vt)) {
        vtfree(vt);
        vtfreetextbuf(&vt->textbuf);
        vtfreetextbuf(&vt->scrbuf);
        vtfreecolors(vt);
        if (newvt) {
            free(vt);
        }
        
        return NULL;
    }

    return vt;
}

#if (VTTEST)
void
vtprintinfo(struct vt *vt)
{
    fprintf(stderr, "VT: fd == %d, master: %s, slave: %s\n",
            vt->atr.fd, vt->atr.masterpath, vt->atr.slavepath);
    fprintf(stderr, "VT: %ld rows x %ld columns\n",
            vt->state.nrow, vt->state.ncol);
    fprintf(stderr, "VT: %ld buffer rows @ %p\n",
            vt->textbuf.nrow, vt->textbuf.data);
    fprintf(stderr, "VT: %ld screen rows @ %p\n",
            vt->scrbuf.nrow, vt->scrbuf.data);

    return;
}

int
main(int argc, char *argv[])
{
    struct vt vt;

    memset(&vt, 0, sizeof(struct vt));
    vt.state.nrow = 24;
    vt.state.ncol = 80;
    vt.textbuf.nrow = VTDEFBUFNROW;
    vt.scrbuf.nrow = 24;
    if (vtinit(&vt, argc, argv)) {
        vtprintinfo(&vt);
    } else {
        fprintf(stderr, "failed to initialise VT\n");
    }
    vt.state.w = vt.state.ncol * vt.font.boxw;
    vt.state.h = vt.state.nrow * vt.font.boxh;

    exit(1);
}
#endif /* VTTEST */

