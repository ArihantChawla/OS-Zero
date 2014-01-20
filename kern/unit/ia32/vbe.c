#include <kern/conf.h>

#if (VBE)

#include <stdint.h>

#include <gfx/rgb.h>
#include <kern/util.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/real.h>
#include <kern/unit/ia32/vbe.h>
#include <kern/unit/ia32/vm.h>

extern void gdtinit(void);
extern void realint10(void);

#define VBEPTR(x) ((((uint32_t)(x) & 0xffffffff) >> 12) | ((uint32_t)(x) & 0xffff))

struct vbe {
    long found;
    long mode;
};

struct vbe              vbe;
struct vbescreen        vbescreen;
static struct vbeinfo   vbectlinfo;

void
vbeint10(struct realregs *regs)
{
    static int first = 1;

    if (first) {
        kmemcpy((void *)KERNREALBASE,
                &realstart,
                (unsigned long)&realend - (unsigned long)&realstart);
        first = 0;
    }
    kmemcpy((void *)(KERNREALSTK - sizeof(struct realregs)),
            regs,
            sizeof(struct realregs));
    realint10();

    return;
}

void
vbeinit(void)
{
    struct realregs  regs;
    struct vbeinfo  *info = (struct vbeinfo *)VBEINFOADR;

    kbzero(info, sizeof(struct vbeinfo));
    regs.ax = VBEGETINFO;
    regs.di = VBEINFOADR;
    info->sig[0] = 'V';
    info->sig[1] = 'B';
    info->sig[2] = 'E';
    info->sig[3] = '2';
    vbeint10(&regs);
    gdtinit();

    return;
}

void
vbeprintinfo(void)
{
    struct vbeinfo *info = (void *)0xa000;
    uint16_t *modeptr = (uint16_t *)VBEPTR(info->modelst);

//    kmemcpy(&vbectlinfo, (void *)0xa000, sizeof(struct vbeinfo));
    modeptr = (uint16_t *)VBEPTR(vbectlinfo.modelst);
    kprintf("VBE modes:");
    while (*modeptr != VBEMODELSTEND) {
        if (*modeptr) {
            kprintf(" %x", *modeptr);
        }
        modeptr++;
    }
    kprintf("\n");

    return;
}

#if 0
long
vbeinit(void)
{
//    struct vbemode *mode = (struct vbemode *)hdr->vbemodeinfo;
    long            bpp = (mode) ? mode->npixbit : 0;
    long            retval;

    retval = (hdr->flags & GRUBVBE);
    if (retval) {
        kprintf("framebuffer @ %x\n", mode->fbadr);
        vbe2screen.fbuf = (void *)mode->fbadr;
        vbe2screen.w = mode->xres;
        vbe2screen.h = mode->yres;
        vbe2screen.nbpp = bpp;
        vbe2screen.fmt = ((bpp == 24)
                           ? GFXRGB888
                           : ((bpp == 16)
                              ? GFXRGB565
                              : GFXRGB555));
        vmmapseg((uint32_t *)&_pagetab,
                 (uint32_t)vbe2screen.fbuf,
                 (uint32_t)vbe2screen.fbuf,
                 (uint32_t)vbe2screen.fbuf
                 + ((bpp == 24)
                    ? mode->xres * mode->yres * 3
                    : mode->xres * mode->yres * 2),
                 PAGEPRES | PAGEWRITE);
    }

    return retval;
}
#endif /* 0 */

#endif /* VBE */

