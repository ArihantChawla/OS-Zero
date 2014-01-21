#include <kern/conf.h>

#if (VBE)

#include <stdint.h>

#include <gfx/rgb.h>
#include <kern/util.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/link.h>
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

struct vbe            vbe;
struct vbescreen      vbescreen;
static struct vbeinfo vbectlinfo;

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
#if 0
    kmemcpy((void *)(KERNREALSTK - sizeof(struct realregs)),
            regs,
            sizeof(struct realregs));
#endif
    realint10();

    return;
}

void
vbeinit(void)
{
    struct realregs *regs = (void *)(KERNREALSTK - sizeof(struct realregs));
    struct vbeinfo  *info = (void *)VBEINFOADR;

    kbzero(regs, sizeof(struct realregs));
    kbzero(info, sizeof(struct vbeinfo));
    regs->ax = VBEGETINFO;
    regs->di = VBEINFOADR;
    info->sig[0] = 'V';
    info->sig[1] = 'B';
    info->sig[2] = 'E';
    info->sig[3] = '2';
    vbeint10(regs);
    gdtinit();
    if (regs->ax != VBESUPPORTED) {

        return;
    }
    kbzero(regs, sizeof(struct realregs));
    regs->ax = VBEGETMODEINFO;
    regs->cx = 0x118;
    regs->di = VBEMODEADR;
    vbeint10(regs);
    gdtinit();
    if (regs->ax != VBESUPPORTED) {

        return;
    }
    kbzero(regs, sizeof(struct realregs));
    regs->ax = VBESETMODE;
    regs->bx = 0x118 | VBELINFBBIT;
    vbeint10(regs);
    gdtinit();
    if (regs->ax != VBESUPPORTED) {

        return;
    }

    /* for testing only */
    kbzero(regs, sizeof(struct realregs));
    regs->ax = 0x4f03;
    vbeint10(regs);
    gdtinit();
    if (regs->ax != VBESUPPORTED) {

        return;
    }



    return;
}

void
vbeinitscr(void)
{
    struct vbemode  *mode = (void *)VBEMODEADR;

    vbescreen.fbuf = (void *)mode->fbadr;
    vbescreen.w = mode->xres;
    vbescreen.h = mode->yres;
    vbescreen.nbpp = mode->npixbit;
// TODO: set vbescreen->fmt
    vbescreen.mode = mode;
    /* identity-map VBE framebuffer */
    vmmapseg((uint32_t *)&_pagetab,
             (uint32_t)vbescreen.fbuf,
             (uint32_t)vbescreen.fbuf,
             (uint32_t)vbescreen.fbuf
             + ((vbescreen.nbpp == 24)
                ? vbescreen.mode->xres * vbescreen.mode->yres * 3
                : vbescreen.mode->xres * vbescreen.mode->yres * 2),
             PAGEPRES | PAGEWRITE);

    return;
}

void
vbeprintinfo(void)
{
    struct vbeinfo *info = (void *)VBEMODEADR;
    uint16_t       *modeptr = (uint16_t *)VBEPTR(info->modelst);

//    kmemcpy(&vbectlinfo, (void *)0xa000, sizeof(struct vbeinfo));
    kprintf("VBE OEM: %s\n", VBEPTR(*((uint32_t *)info->oem)));
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

