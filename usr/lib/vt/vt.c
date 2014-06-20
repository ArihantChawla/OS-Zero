#include <vt/conf.h>
#if (VTTEST)
#include <stdio.h>
#endif
#include <stdlib.h>
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
#if 0 /* TODO: initialise textbuffers in term.c */
    nrow = vt->textbuf.nrow;
    if (!nrow) {
        nrow = VTDEFBUFNROW;
    }
    ncol = vt->state.ncol;
    if (!ncol) {
        ncol = VTDEFNCOL;
        vt->state.ncol = ncol;
    }
    if (!uiinittextbuf(&vt->textbuf, nrow, ncol)) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
    nrow = vt->state.nrow;
    if (!nrow) {
        nrow = VTDEFNROW;
        vt->state.nrow = nrow;
    }
    if (!uiinittextbuf(&vt->scrbuf, nrow, ncol)) {
        vtfree(vt);
        if (newvt) {
            free(vt);
        }

        return NULL;
    }
#endif
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

