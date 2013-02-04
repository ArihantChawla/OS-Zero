#include <kern/conf.h>

#if (VBE2)

#include <gfx/rgb.h>
#include <kern/util.h>
#include <kern/io/drv/pc/vbe2.h>

static struct vbescreen _vbe2screen;

long
vbe2init(struct mboothdr *hdr)
{
    struct vbemode *mode = (struct vbemode *)hdr->vbemodeinfo;
    long            bpp = (mode) ? mode->npixbit : 0;
    long            retval = 0;

    retval = (hdr->flags & GRUBVBE);
    if (retval) {
        _vbe2screen.fbuf = (void *)mode->fbadr;
        _vbe2screen.w = mode->xres;
        _vbe2screen.h = mode->yres;
        _vbe2screen.nbpp = bpp;
        _vbe2screen.fmt = ((bpp == 24)                                  \
                           ? GFXRGB888                                  \
                           : ((bpp == 16)                               \
                              ? GFXRGB565                               \
                              : GFXRGB555));
    }

    return retval;
}

void
vbe2printinfo(struct mboothdr *hdr)
{
    struct vbeinfo *ctl = (struct vbeinfo *)hdr->vbectlinfo;
    struct vbemode *mode = (struct vbemode *)hdr->vbemodeinfo;

    if (hdr->flags & GRUBVBE) {
        kprintf("VBE version %x\n", ctl->ver);
        kprintf("VBE %dx%d@%d mode: %x\n", mode->xres, mode->yres,
                mode->npixbit, hdr->vbemode);
        kprintf("VBE fb @ %p\n", mode->fbadr);
    }

    return;
}

#endif /* VBE2 */

