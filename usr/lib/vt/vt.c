#if (VTTEST)
#include <stdio.h>
#endif
#include <stdlib.h>
#include <limits.h>
#include <vt/vt.h>
#include <vt/pty.h>

void
vtfree(struct vt *vt, long newvt)
{
    if (vt->inbuf.base) {
        free(vt->inbuf.base);
        vt->inbuf.base = NULL;
    }
    if (vt->outbuf.base) {
        free(vt->outbuf.base);
        vt->outbuf.base = NULL;
    }
    if (vt->masterpath) {
        free(vt->masterpath);
        vt->masterpath = NULL;
    }
    if (vt->slavepath) {
        free(vt->slavepath);
        vt->masterpath = NULL;
    }
    if (newvt) {
        free(vt);
    }
}

struct vt *
vtinit(struct vt *vt)
{
    long  newvt = (vt) ? 0 : 1;

    if (!vt) {
        vt = malloc(sizeof(struct vt));
        if (!vt) {

            return vt;
        }
    }
    if (!ringinit(&vt->inbuf, NULL, VTBUFSIZE / sizeof(RING_ITEM))
        || !ringinit(&vt->inbuf, NULL, VTBUFSIZE / sizeof(RING_ITEM))
        || !(vt->masterpath = malloc(PATH_MAX))
        || !(vt->slavepath = malloc(PATH_MAX))) {
        vtfree(vt, newvt);
        
        return NULL;
    }
    vt->fd = vtopenpty_posix(&vt->masterpath, &vt->slavepath);
    if (vt->fd < 0) {
        vtfree(vt, newvt);

        return NULL;
    }
    vt->mode = VTDEFMODE;
    vt->state = 0;
    vt->fgcolor = VTDEFFGCOLOR;
    vt->bgcolor = VTDEFBGCOLOR;
    vt->textatr = VTDEFTEXTATR;

    return vt;
}

#if (VTTEST)
void
vtprintinfo(struct vt *vt)
{
    fprintf(stderr, "VT(%d): %s, %s\n", vt->fd, vt->masterpath, vt->slavepath);

    return;
}

int
main(int argc, char *argv[])
{
    struct vt *vt = vtinit(NULL);

    if (vt) {
        vtprintinfo(vt);
    } else {
        fprintf(stderr, "failed to initialise VT\n");
    }

    exit(1);
}
#endif
