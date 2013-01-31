#include <stdint.h>
#include <stddef.h>
#include <zero/cdecl.h>
#include <kern/conf.h>
#include <kern/util.h>
#include <kern/io/drv/pc/vga.h>

struct vgacon  _vgacontab[VGANCON] ALIGNED(PAGESIZE);
static long    _vgacurcon;

#if (VGAGFX)

/* copy VGA font glyph data to VGAFONTADR */
void
vgagetfont(void)
{
    __asm__ __volatile__ ("movw $0x05, %%ax\n"          // clear even/odd mode
                          "movw $0x03ce, %%dx\n"
                          "outw %%ax, %%dx\n"
                          "movw $0x0406, %%ax\n"        // map VGA to 0xa0000
                          "outw %%ax, %%dx\n"
                          "movw $0x03c4, %%dx\n"        // set bitplane 2
                          "movw $0x0402, %%ax\n"
                          "outw %%ax, %%dx\n"           // clear even/odd mode
                          "movw $0x0704, %%ax\n"
                          "outw %%ax, %%dx\n"
                          "movl %c0, %%esi\n"           // glyph address
                          "movl %c1, %%ecx\n"           // glyph count
                          "_copy: movsd\n"              // copy glyph map
                          "movsd\n"
                          "movsd\n"
                          "movsd\n"
                          "addl $16, %%esi\n"
                          "loop _copy\n"
                          "movw $0x0302, %%ax\n"        // restore VGA to normal
                          "outw %%ax, %%dx\n"
                          "movw $0x0304, %%ax\n"
                          "outw %%ax, %%dx\n"
                          "movw $0x03ce, %%dx\n"
                          "movw $0x1005, %%ax\n"
                          "outw %%ax, %%dx\n"
                          "movw $0x0a06, %%ax\n"
                          "outw %%ax, %%dx\n"
                          :
                          : "i" (VGAFONTADR), "i" (VGANGLYPH)
                          : "eax", "edx", "edi", "esi");

    return;
}

void
vgaputpix(int32_t fg, int32_t x, int32_t y)
{
    ;
}

#endif /* VGAGFX */

/* initialise 8 consoles */
void
vgainitcon(int w, int h)
{
    struct vgacon *con = _vgacontab;
    uint8_t       *ptr = (uint8_t *)VGABUFADR;
    long           l;

#if (VGAGFX)
    vgagetfont();
#endif
    for (l = 0 ; l < VGANCON ; l++) {
        bzero(ptr, PAGESIZE);
#if (VGAGFX)
        con->fg = 0xffffffff;
#else
        con->buf = (uint16_t *)ptr;
#endif
        con->x = 0;
        con->y = 0;
        con->w = w;
        con->h = h;
#if (!VGAGFX)
        con->chatr = vgasetfg(0, VGAWHITE);
#endif
        con->nbufln = 0;
        /* TODO: allocate scrollback buffer */
        con->data = NULL;
        ptr += VGABUFSIZE;
        con++;
    }
    _vgacurcon = 0;
    vgamoveto(0, 0);
#if 0
    kprintf("VGA @ 0x%x - width = %d, height = %d, %d consoles\n",
            VGABUFADR, w, h, VGANCON);
#endif

    return;
}

void
vgaputs(char *str)
{
    struct vgacon *con;
#if (!VGAGFX)
    uint16_t      *ptr;
#endif
    int            x;
    int            y;
    int            w;
    int            h;
    uint8_t        ch;
#if (!VGAGFX)
    uint8_t        atr;
#endif

    con = &_vgacontab[_vgacurcon];
    x = con->x;
    y = con->y;
    w = con->w;
    h = con->h;
#if (!VGAGFX)
    atr = con->chatr;
#endif
    while (*str) {
#if (!VGAGFX)
        ptr = con->buf + y * w + x;
#endif
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
#if (VGAGFX)
            vgadrawchar(ch, (x << 3), (y << 3), con->fg, con->bg);
#else
            vgaputch3(ptr, ch, atr);
#endif
        }
        str++;
        con->x = x;
        con->y = y;
    }

    return;
}

void
vgaputs2(long conid, char *str)
{
    struct vgacon *con;
#if (!VGAGFX)
    uint16_t      *ptr;
#endif
    int            x;
    int            y;
    int            w;
    int            h;
    uint8_t        ch;
#if (!VGAGFX)
    uint8_t        atr;
#endif

    con = &_vgacontab[conid];
    x = con->x;
    y = con->y;
    w = con->w;
    h = con->h;
#if (!VGAGFX)
    atr = con->chatr;
#endif
    while (*str) {
#if (!VGAGFX)
        ptr = con->buf + y * w + x;
#endif
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
#if (VGAGFX)
            vgadrawchar(ch, (x << 3), (y << 3), con->fg, con->bg);
#else
            vgaputch3(ptr, ch, atr);
#endif
        }
        str++;
        con->x = x;
        con->y = y;
    }

    return;
}

void
vgaputchar(int ch)
{
    struct vgacon *con;
#if (!VGAGFX)
    uint16_t      *ptr;
#endif

    con = &_vgacontab[_vgacurcon];
#if (VGAGFX)
    vgadrawchar(ch, (con->x << 3), (con->y << 3), con->fg, con->bg);
#else
    ptr = con->buf + con->w * con->x + con->y;
    *ptr = _vgamkch(ch, con->chatr);
#endif

    return;
}

void
vgasyncscr(void)
{
    ;
}

