#include <kern/conf.h>

#if (VBE2)

#include <gfx/rgb.h>
#include <kern/util.h>
#include <kern/io/drv/pc/vbe2.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/vm.h>

struct vbescreen vbe2screen;

long
vbe2init(struct mboothdr *hdr)
{
    struct vbemode *mode = (struct vbemode *)hdr->vbemodeinfo;
    long            bpp = (mode) ? mode->npixbit : 0;
    long            retval = 0;

    retval = (hdr->flags & GRUBVBE);
    if (retval) {
        vbe2screen.fbuf = (void *)mode->fbadr;
        vbe2screen.w = mode->xres;
        vbe2screen.h = mode->yres;
        vbe2screen.nbpp = bpp;
        vbe2screen.fmt = ((bpp == 24)
                           ? GFXRGB888
                           : ((bpp == 16)
                              ? GFXRGB565
                              : GFXRGB555));
        vmmapseg((uint32_t *)&_pagetab, vbe2screen.fbuf, vbe2screen.fbuf,
                 (uint32_t)vbe2screen.fbuf
                 + ((bpp * mode->xres * mode->yres >> 3)),
                 PAGEPRES | PAGEWRITE);
        bfill(vbe2screen.fbuf, 0x7f, bpp * mode->xres * mode->yres);
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

