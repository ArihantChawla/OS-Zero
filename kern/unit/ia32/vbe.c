#include <kern/conf.h>

#if (VBE)

#include <stdint.h>

#include <gfx/rgb.h>
#include <kern/util.h>
#include <kern/io/drv/chr/con.h>
#include <kern/io/drv/pc/vga.h>
#include <kern/unit/ia32/boot.h>
#include <kern/unit/ia32/link.h>
#include <kern/unit/ia32/real.h>
#include <kern/unit/ia32/vbe.h>
#include <kern/unit/ia32/vm.h>

extern void  realint10(void);

extern void *vgafontbuf;

void vbeputs(char *str);
void vbeputchar(int ch);

void vbedrawchar(unsigned char c, int x, int y, argb32_t fg, argb32_t bg);

#define VBEPTR(x) ((((uint32_t)(x) & 0xffffffff) >> 12) | ((uint32_t)(x) & 0xffff))

#if 0
struct vbe {
    long found;
    long mode;
};
#endif

#if 0
struct vbe            vbe;
#endif
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
    gdtinit();

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
    if (regs->ax != VBESUPPORTED) {

        return;
    }
    kbzero(regs, sizeof(struct realregs));
    regs->ax = VBEGETMODEINFO;
    regs->cx = 0x118;
    regs->di = VBEMODEADR;
    vbeint10(regs);
    if (regs->ax != VBESUPPORTED) {

        return;
    }
    kbzero(regs, sizeof(struct realregs));
    regs->ax = VBESETMODE;
    regs->bx = 0x118 | VBELINFBBIT;
    vbeint10(regs);
    if (regs->ax != VBESUPPORTED) {

        return;
    }

    return;
}

void
vbehello(void)
{
    vbedrawchar('h', 0, 0, 0x00ffffff, 0x00000000);
    vbedrawchar('e', 8, 0, 0x00ffffff, 0x00000000);
    vbedrawchar('l', 16, 0, 0x00ffffff, 0x00000000);
    vbedrawchar('l', 24, 0, 0x00ffffff, 0x00000000);
    vbedrawchar('o', 32, 0, 0x00ffffff, 0x00000000);

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
    vbehello();

    return;
}

void
vbeinitcon(int w, int h)
{
    struct con *con = contab;
    long        l;

    for (l = 0 ; l < VGANCON ; l++) {
//        kbzero(ptr, PAGESIZE);
        con->puts = vbeputs;
        con->putchar = vbeputchar;
        con->fg = RGBWHITE;
        con->bg = RGBBLACK;
        con->buf = vbescreen.fbuf;
        con->x = 0;
        con->y = 0;
        con->w = w;
        con->h = h;
//        con->chatr = vgasetfg(0, VGAWHITE);
        con->nbufln = 0;
        /* TODO: allocate scrollback buffer */
        con->data = NULL;
        con++;
    }
    concur = 0;

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

void
vbeclrscr(uint32_t pix)
{
    long x;
    long y;

    for (x = 0 ; x < vbescreen.w ; x++) {
        for (y = 0 ; y < vbescreen.h ; y++) {
            vbeputpix(pix, x, y);
        }
    }

    return;
}

void
vbeputpix(argb32_t pix, int x, int y)
{
    uint8_t *_ptr = vbepixadr(x, y);

    gfxsetrgb888(pix, _ptr);

    return;
}

void
vbedrawchar(unsigned char c, int x, int y, argb32_t fg, argb32_t bg)
{
    long     cy;
    long     incr = vbescreen.w * (vbescreen.nbpp >> 3);
    uint8_t *glyph = (uint8_t *)vgafontbuf + ((int)c << 3);
    uint8_t *ptr = vbepixadr(x, y);
    uint8_t  mask;
    

    for (cy = 0 ; cy < VGAGLYPHH >> 1 ; cy++) {
        mask = *glyph;
        if (mask & 0x80) {
            gfxsetrgb888(fg, ptr);
        }
        if (mask & 0x40) {
            gfxsetrgb888(fg, ptr + 3);
        }
        if (mask & 0x20) {
            gfxsetrgb888(fg, ptr + 6);
        }
        if (mask & 0x10) {
            gfxsetrgb888(fg, ptr + 9);
        }
        if (mask & 0x08) {
            gfxsetrgb888(fg, ptr + 12);
        }
        if (mask & 0x04) {
            gfxsetrgb888(fg, ptr + 15);
        }
        if (mask & 0x02) {
            gfxsetrgb888(fg, ptr + 18);
        }
        if (mask & 0x01) {
            gfxsetrgb888(fg, ptr + 21);
        }
        glyph++;
        ptr += incr;
    }

    return;
}

void
vbedrawcharbg(unsigned char c, int x, int y, argb32_t fg, argb32_t bg)
{
    long     cy;
    long     incr = vbescreen.w * (vbescreen.nbpp >> 3);
    uint8_t *glyph = (uint8_t *)vgafontbuf + ((int)c << 3);
    uint8_t *ptr = vbepixadr(x, y);
    uint8_t  mask;
    

    for (cy = 0 ; cy < VGAGLYPHH >> 1 ; cy++) {
        mask = *glyph;
        gfxsetrgb888((mask & 0x80) ? fg : bg, ptr);
        gfxsetrgb888((mask & 0x40) ? fg : bg, ptr + 3);
        gfxsetrgb888((mask & 0x20) ? fg : bg, ptr + 6);
        gfxsetrgb888((mask & 0x10) ? fg : bg, ptr + 9);
        gfxsetrgb888((mask & 0x08) ? fg : bg, ptr + 12);
        gfxsetrgb888((mask & 0x04) ? fg : bg, ptr + 15);
        gfxsetrgb888((mask & 0x02) ? fg : bg, ptr + 18);
        gfxsetrgb888((mask & 0x01) ? fg : bg, ptr + 21);
        glyph++;
        ptr += incr;
    }

    return;
}

/* output string on the current console */
void
vbeputs(char *str)
{
    struct con *con;
    uint16_t   *ptr;
    int         x;
    int         y;
    int         w;
    int         h;
    uint8_t     ch;
//    uint8_t   atr;

    con = &contab[concur];
    x = con->x;
    y = con->y;
    w = con->w;
    h = con->h;
//    atr = con->chatr;
    while (*str) {
        ptr = con->buf + y * w + x;
        ch = *str;
        if (ch == '\n') {
            if (++y == h) {
                y = 0;
            }
            x = 0;
        } else {
            if (++x == w) {
                x = 0;
                if (++y == h) {
                    y = 0;
                }
            }
            vbedrawchar(ch, x << 3, y << 3, con->fg, con->bg);
        }
        str++;
        con->x = x;
        con->y = y;
    }

    return;
}

/* output string on a given console */
void
vbeputs2(struct con *con, char *str)
{
    uint16_t      *ptr;
    int            x;
    int            y;
    int            w;
    int            h;
    uint8_t        ch;
    uint8_t        atr;

    x = con->x;
    y = con->y;
    w = con->w;
    h = con->h;
    atr = con->chatr;
    while (*str) {
        ptr = con->buf + y * w + x;
        ch = *str;
        if (ch == '\n') {
            if (++y == h) {
                y = 0;
            }
            x = 0;
        } else {
            if (++x == w) {
                x = 0;
                if (++y == h) {
                    y = 0;
                }
            }
            vbedrawchar(ch, x << 3, y << 3, con->fg, con->bg);
        }
        str++;
        con->x = x;
        con->y = y;
    }

    return;
}

void
vbeputchar(int ch)
{
    struct con *con;
    uint16_t      *ptr;

    con = &contab[concur];
    vbedrawchar(ch, (con->x << 3), (con->y << 3), con->fg, con->bg);

    return;
}

#if 0
/* draw character without background (transparent) */
void
vbedrawcharfg(unsigned char c, int x, int y, argb32_t fg, argb32_t bg)
{
    int      _cy;
    int      _yofs;
    uint8_t *_gp = (uint8_t *)vgafontbuf + ((int)c << 4);
    uint8_t  _g;

    for (_cy = 0 ; _cy < VGAGLYPHH >> 1; _cy++) {
        _g = *_gp;
        _yofs = y + _cy;
        if (_g & 0x01) {
            vbeputpix(fg, x + 7, _yofs);
        }
        if (_g & 0x02) {
            vbeputpix(fg, x + 6, _yofs);
        }
        if (_g & 0x04) {
            vbeputpix(fg, x + 5, _yofs);
        }
        if (_g & 0x08) {
            vbeputpix(fg, x + 4, _yofs);
        }
        if (_g & 0x10) {
            vbeputpix(fg, x + 3, _yofs);
        }
        if (_g & 0x20) {
            vbeputpix(fg, x + 2, _yofs);
        }
        if (_g & 0x40) {
            vbeputpix(fg, x + 1, _yofs);
        }
        if (_g & 0x80) {
            vbeputpix(fg, x + 0, _yofs);
        }
        _gp++;
    }

    return;
}
#endif
        
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
        /* map VBE framebuffer */
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

