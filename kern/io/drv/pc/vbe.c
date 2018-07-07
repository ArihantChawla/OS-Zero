#include <kern/conf.h>

#if (VBE)

#define VBEDEFMODE 0x118        // 1024x768 24-bit

#include <stdint.h>
#include <zero/cdefs.h>
#if (NEWFONT)
#include <zero/trix.h>
#endif
#include <zero/types.h>
#include <gfx/rgb.h>
#include <kern/malloc.h>
#include <kern/util.h>
#if (SMP)
#include <kern/cpu.h>
#endif
#include <kern/mem/vm.h>
#if (VBEMTRR)
#include <kern/mem/mtrr.h>
#endif
#include <kern/io/drv/chr/cons.h>
#include <kern/io/drv/pc/vga.h>
#include <kern/io/drv/pc/vbe.h>
#include <kern/unit/x86/boot.h>
#include <kern/unit/ia32/real.h>

extern void            realint10(void);

extern struct m_farptr realgdtptr;
#if (SMP)
extern uint64_t        kerngdt[NCPU][NGDT];
#else
extern uint64_t        kerngdt[NGDT];
#endif
extern FASTCALL void gdtinit(struct m_farptr *farptr);

extern void *vgafontbuf;
#if (NEWFONT)
long         vbefontw = 7;
long         vbefonth = 13;
#endif

//void vbeputpix(gfxpix32 pix, int x, int row);
void vbeputpix(gfxpix32 pix, uint8_t *ptr);
void vbeputs(char *str);
void vbeputchar(int ch);

#define VBEPTR(x)                                                       \
    ((((uint32_t)(x) & 0xffffffff) >> 12) | ((uint32_t)(x) & 0xffff))

struct vbescreen      vbescreen;
static struct vbeinfo vbectlinfo;

void
vbeint10(void)
{
#if (SMP)
    long id = k_curcpu->id;
#endif
    static int       first = 1;
    uint64_t        *gdt;
    struct m_farptr *farptr;
    if (first) {
        kmemcpy((void *)BOOTREALBASE,
                &realstart,
                (unsigned long)&realend - (unsigned long)&realstart);
        first = 0;
    }
    realint10();
#if (SMP)
    gdt = &kerngdt[id][0];
#else
    gdt = kerngdt;
#endif
    farptr = &realgdtptr;
    farptr->lim = NGDT * sizeof(uint64_t) - 1;
    farptr->adr = (uint32_t)gdt;
    gdtinit(farptr);

    return;
}

/*
 * initialise VBE graphics
 * - probe the controller
 * - probe the modes
 * - set the default mode
 */
void
vbeinit(void)
{
    struct realregs *regs = (void *)(BOOTREALSTK - sizeof(struct realregs));
    struct vbeinfo  *info = (void *)VBEINFOADR;

    kbzero(regs, sizeof(struct realregs));
    kbzero(info, sizeof(struct vbeinfo));
    regs->ax = VBEGETINFO;
    regs->di = VBEINFOADR;
    info->sig[0] = 'V';
    info->sig[1] = 'B';
    info->sig[2] = 'E';
    info->sig[3] = '2';
    vbeint10();
    if (regs->ax != VBESUPPORTED) {

        return;
    }
    kbzero(regs, sizeof(struct realregs));
    regs->ax = VBEGETMODEINFO;
    regs->cx = VBEDEFMODE;
    regs->di = VBEMODEADR;
    vbeint10();
    if (regs->ax != VBESUPPORTED) {

        return;
    }
    kbzero(regs, sizeof(struct realregs));
    regs->ax = VBESETMODE;
    regs->bx = VBEDEFMODE | VBELINFBBIT;
    vbeint10();
    if (regs->ax != VBESUPPORTED) {

        return;
    }

    return;
}

void
vbeinitscr(void)
{
    struct vbemode *mode = (void *)VBEMODEADR;
    long            npix;

    vbescreen.mode = mode;
    vbescreen.nbpp = mode->npixbit;
    npix = mode->xres * mode->yres;
    if (vbescreen.nbpp == 32) {
        vbescreen.pixsize = 4;
        vbescreen.fbufsize = npix << 2;
    } else if (vbescreen.nbpp == 24) {
        vbescreen.pixsize = 3;
        vbescreen.fbufsize = npix + (npix << 1);
    } else if (vbescreen.nbpp == 15
               || vbescreen.nbpp == 16) {
        vbescreen.pixsize = 2;
        vbescreen.fbufsize = npix << 1;
    } else if (vbescreen.nbpp == 8) {
        vbescreen.pixsize = 2;
        vbescreen.fbufsize = npix;
    }
    vbescreen.fbuf = (void *)mode->fbadr;
    vbescreen.w = mode->xres;
    vbescreen.h = mode->yres;
// TODO: set vbescreen->fmt
    /* identity-map VBE framebuffer */
    vmmapseg((uint32_t)0xa000,
             (uint32_t)0xa000,
             (uint32_t)0xa000 + sizeof(struct vbeinfo),
             PAGEPRES | PAGEWRITE | PAGENOCACHE | PAGEWIRED);
    vmmapseg((uint32_t)vbescreen.fbuf,
             (uint32_t)vbescreen.fbuf,
             (uint32_t)vbescreen.fbuf + vbescreen.fbufsize,
             PAGEPRES | PAGEWRITE | PAGENOCACHE | PAGEWIRED);
#if (VBEMTRR)
    mtrrsetwrcomb((uint32_t)vbescreen.fbuf, vbescreen.fbufsize);
#endif
    vbeclrscr(GFX_BLACK);

    return;
}

void
vbeinitcons(int w, int h)
{
    struct cons  *cons = constab;
    conschar_t  **buf;
    void         *ptr;
    long          bufsz = CONSNBUFROW * sizeof(conschar_t *);
    long          rowsz = (w + 1) * sizeof(conschar_t);
    long          l;
    long          row;
    long          n = 0;

    for (l = 0 ; l < NCONS ; l++) {
        cons->puts = vbeputs;
        cons->putchar = vbeputchar;
        cons->fg = GFX_WHITE;
        cons->bg = GFX_BLACK;
        cons->buf = vbescreen.fbuf;
        cons->col = 0;
        cons->row = 0;
        cons->ncol = w;
        cons->nrow = h;
        cons->nbufrow = CONSNBUFROW;
#if 0
        /* TODO: allocate scrollback buffer */
        buf = kcalloc(bufsz);
        if (!buf) {
            kprintf("CONS: failed to allocate console row buffer\n");

            panic(0, 0);
        }
        n++;
        cons->textbuf = buf;
        for (row = 0 ; row < CONSNBUFROW ; row++) {
            /* allocate NUL-terminated row */
            ptr = kcalloc(rowsz);
            if (!ptr) {
                kprintf("CONS %l: failed to allocate console row %l (%l)\n",
                        l, row, n);

                panic(0, 0);
            }
            n++;
            buf[row] = ptr;
        }
#endif
        cons++;
    }
    conscur = 0;

    return;
}

void
vbeprintinfo(void)
{
    struct vbeinfo *info = (void *)VBEMODEADR;
    uint16_t       *modeptr = (uint16_t *)VBEPTR(info->modelst);
    long            cnt;

//    kmemcpy(&vbectlinfo, (void *)0xa000, sizeof(struct vbeinfo));
    kprintf("VBE FB: %ld kilobytes @ 0x%p - 0x%p\n",
            vbescreen.fbufsize >> 10, vbescreen.fbuf,
            (void *)((uint8_t *)vbescreen.fbuf + vbescreen.fbufsize - 1));
    kprintf("VBE OEM: %s\n", (void *)VBEPTR(&info->oem));
    modeptr = (uint16_t *)VBEPTR(vbectlinfo.modelst);
    kprintf("VBE modes:");
    cnt = 0;
    while (*modeptr != VBEMODELSTEND) {
        if (*modeptr) {
            kprintf(" %x", *modeptr);
        }
        cnt++;
        if (cnt == 16) {
            kprintf("\n");
            cnt = 0;
        }
        modeptr++;
    }
    kprintf("\n");

    return;
}

INLINE void
vbeputpix(gfxpix32 pix, uint8_t *ptr)
{
    gfxsetrgb888(pix, ptr);

    return;
}

void
vbeclrscr(gfxpix32 pix)
{
    uint8_t *ptr = vbepixadr(0, 0);
    long     incr = vbescreen.pixsize;
    long     n = vbescreen.w * vbescreen.h;
    long     nbpp = vbescreen.nbpp;
    long     lim;

    if (nbpp == 24) {
        while (n) {
            lim = min(n, 8);
            switch (lim) {
                case 8:
                    vbeputpix(pix, &ptr[7 * incr]);
                case 7:
                    vbeputpix(pix, &ptr[6 * incr]);
                case 6:
                    vbeputpix(pix, &ptr[5 * incr]);
                case 5:
                    vbeputpix(pix, &ptr[4 * incr]);
                case 4:
                    vbeputpix(pix, &ptr[3 * incr]);
                case 3:
                    vbeputpix(pix, &ptr[2 * incr]);
                case 2:
                    vbeputpix(pix, &ptr[incr]);
                case 1:
                    vbeputpix(pix, ptr);
                case 0:
                default:

                    break;
            }
            n -= lim;
            ptr += lim * incr;
        }
    }

    return;
}

#if (NEWFONT)

void
vbedrawchar(unsigned char c, int x, int y, gfxpix32 fg)
{
    long      lim = vbefonth;
    long      cy;
    long      incr = vbescreen.w * (vbescreen.nbpp >> 3);
//    uint8_t *glyph = (uint8_t *)vgafontbuf + ((int)c * vbefonth);
    uint16_t *glyph = (uint16_t *)vgafontbuf + (int)c * vbefonth;
    uint8_t  *ptr = vbepixadr(x, y);
    uint16_t  mask;

    for (cy = 0 ; cy < lim ; cy++) {
        mask = *glyph;
#if 0
        if (mask & 0x80) {
            gfxsetrgb888(fg, ptr);
        }
#endif
        if (mask & 0x40) {
            gfxsetrgb888(fg, ptr);
        }
        if (mask & 0x20) {
            gfxsetrgb888(fg, &ptr[3]);
        }
        if (mask & 0x10) {
            gfxsetrgb888(fg, &ptr[6]);
        }
        if (mask & 0x08) {
            gfxsetrgb888(fg, &ptr[9]);
        }
        if (mask & 0x04) {
            gfxsetrgb888(fg, &ptr[12]);
        }
        if (mask & 0x02) {
            gfxsetrgb888(fg, &ptr[15]);
        }
        if (mask & 0x01) {
            gfxsetrgb888(fg, &ptr[18]);
        }
        glyph++;
        ptr += incr;
    }

    return;
}

void
vbedrawcharbg(unsigned char c, int x, int y, gfxpix32 fg, gfxpix32 bg)
{
    long      lim = vbefonth;
    long      cy;
    long      incr = vbescreen.w * (vbescreen.nbpp >> 3);
    uint16_t *glyph = (uint16_t *)vgafontbuf + (int)c * vbefonth;
    uint8_t  *ptr = vbepixadr(x, y);
    uint32_t  val;
    uint32_t  mask;
    gfxpix32  pix;

    if (!bg) {
        for (cy = 0 ; cy < lim ; cy++) {
            val = *glyph;
            pix = fg;
            mask = -(val & 0x40);
            pix &= mask;
            gfxsetrgb888(pix, ptr);
            pix = fg;
            mask = -(val & 0x20);
            pix &= mask;
            gfxsetrgb888(pix, &ptr[3]);
            pix = fg;
            mask = -(val & 0x10);
            pix &= mask;
            gfxsetrgb888(pix, &ptr[6]);
            pix = fg;
            mask = -(val & 0x08);
            pix &= mask;
            gfxsetrgb888(pix, &ptr[9]);
            pix = fg;
            mask = -(val & 0x04);
            pix &= mask;
            gfxsetrgb888(pix, &ptr[12]);
            pix = fg;
            mask = -(val & 0x02);
            pix &= mask;
            gfxsetrgb888(pix, &ptr[15]);
            pix = fg;
            mask = -(val & 0x01);
            pix &= mask;
            gfxsetrgb888(pix, &ptr[18]);
            ptr += incr;
            glyph++;
        }
    } else {
        for (cy = 0 ; cy < lim ; cy++) {
            mask = *glyph;
            if (mask & 0x40) {
                gfxsetrgb888(fg, ptr);
            } else {
                gfxsetrgb888(bg, ptr);
            }
            if (mask & 0x20) {
                gfxsetrgb888(fg, &ptr[3]);
            } else {
                gfxsetrgb888(bg, &ptr[3]);
            }
            if (mask & 0x10) {
                gfxsetrgb888(fg, &ptr[6]);
            } else {
                gfxsetrgb888(bg, &ptr[6]);
            }
            if (mask & 0x08) {
                gfxsetrgb888(fg, &ptr[9]);
            } else {
                gfxsetrgb888(bg, &ptr[9]);
            }
            if (mask & 0x04) {
                gfxsetrgb888(fg, &ptr[12]);
            } else {
                gfxsetrgb888(bg, &ptr[12]);
            }
            if (mask & 0x02) {
                gfxsetrgb888(fg, &ptr[15]);
            } else {
                gfxsetrgb888(bg, &ptr[15]);
            }
            if (mask & 0x01) {
                gfxsetrgb888(fg, &ptr[18]);
            } else {
                gfxsetrgb888(bg, &ptr[18]);
            }
            ptr += incr;
            glyph++;
        }
    }

    return;
}

#else

void
vbedrawchar(unsigned char c, int x, int y, gfxpix32 fg)
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
vbedrawcharbg(unsigned char c, int x, int y, gfxpix32 fg, gfxpix32 bg)
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

#endif

/* output string on a given console */
void
vbeputs2(struct cons *cons, char *str)
{
//    uint16_t *ptr;
    int       x;
    int       row;
    int       w;
    int       h;
    uint8_t   ch;
//    uint8_t   atr;

    x = cons->col;
    row = cons->row;
    w = cons->ncol;
    h = cons->nrow;
//    atr = cons->chatr;
    while (*str) {
//        ptr = cons->buf + row * w + x;
        ch = *str;
        if (ch == '\n') {
            if (++row == h) {
                row = 0;
            }
            x = 0;
        } else {
#if (NEWFONT)
#if (PLASMA)
            vbedrawcharbg(ch, x * vbefontw, row * vbefonth, cons->fg, cons->bg);
#else
            vbedrawchar(ch, x * vbefontw, row * vbefonth, cons->fg);
#endif
#else
            vbedrawchar(ch, x << 3, row << 3, cons->fg);
#endif
            if (++x == w) {
                x = 0;
                if (++row == h) {
                    row = 0;
                }
            }
        }
        str++;
        cons->col = x;
        cons->row = row;
    }

    return;
}

/* output string on the current console */
void
vbeputs(char *str)
{
    vbeputs2(&constab[conscur], str);

    return;
}

void
vbeputchar(int ch)
{
    struct cons *cons;
    long         row;
    long         col;
//    uint16_t    *ptr;

    cons = &constab[conscur];
    row = cons->row;
    col = cons->col;
#if (NEWFONT)
#if (PLASMA)
    vbedrawcharbg(ch, col * vbefontw, row * vbefonth, cons->fg, cons->bg);
#else
    vbedrawchar(ch, col * vbefontw, row * vbefonth, cons->fg);
#endif
#else
    vbedrawchar(ch, col << 3, row << 3, cons->fg);
#endif

    return;
}

#endif /* VBE */

