#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <signal.h>
#include <unistd.h>
#include <vt/term.h>
#include <ui/ui.h>
#include <ui/text.h>
#include <vt/vt.h>

#if (VTTEST)
void
vtprintinfo(struct vt *vt)
{
    fprintf(stderr, "VT: fd == %d, master: %s, slave: %s\n",
            vt->atr.fd, vt->atr.masterpath, vt->atr.slavepath);
    fprintf(stderr, "VT: %ld rows x %ld columns\n",
            vt->state.nrow, vt->state.ncol);
    fprintf(stderr, "VT: %ld buffer rows\n",
            vt->textbuf.nbufrow);
    fprintf(stderr, "VT: %ld screen rows\n",
            vt->textbuf.nrow);
    fprintf(stderr, "VT: font %s (%ldx%ld)\n",
            vt->font.name, vt->font.boxw, vt->font.boxh);

    return;
}
#endif /* VTTEST */

int
main(int argc, char *argv[])
{
    struct vt  vt ALIGNED(CLSIZE);
//    struct vt *vt;
    long       nrow;
    long       ncol;
//    long       nbufrow;

    memset(&vt, 0, sizeof(struct vt));
    vtgetopt(&vt, argc, argv);
    if (!vtinit(&vt, argc, argv)) {
        fprintf(stderr, "failed to initialise VT\n");

        exit(1);
    }
#if 0
    nbufrow = vt.textbuf.nrow;
    if (!nbufrow) {
        nbufnrow = VTDEFBUFNROW;
    }
#endif
    ncol = vt.state.ncol;
    if (!ncol) {
        ncol = VTDEFNCOL;
        vt.state.ncol = ncol;
    }
    nrow = vt.state.nrow;
    if (!nrow) {
        nrow = VTDEFNROW;
        vt.state.nrow = nrow;
    }
    if (!uiinittextbuf(&vt.textbuf, nrow, ncol, 0)) {
        vtfree(&vt);

        exit(1);
    }
    vtprintinfo(&vt);
#if 0
    vt.state.nrow = 24;
    vt.state.ncol = 80;
    vt.textbuf.nrow = VTDEFBUFNROW;
    vt.textbuf.nrow = 24;
#endif
    vt.state.w = vt.state.ncol * vt.font.boxw;
    vt.state.h = vt.state.nrow * vt.font.boxh;
    vtfree(&vt);

    exit(1);
}

