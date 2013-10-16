#include <stdint.h>
#include <stddef.h>
#include <zero/cdecl.h>
#include <kern/conf.h>

#if (!VBE2)

#include <kern/util.h>
#include <kern/io/drv/pc/vga.h>

struct vgacon  _vgacontab[VGANCON] ALIGNED(PAGESIZE);
long           vgacurcon;
static void   *_vgafontbuf;

void
vgainitgfx(void)
{
    ;
}

/* copy font from VGA RAM */
void
vgagetfont(void)
{
    _vgafontbuf = kwalloc(VGAFONTSIZE);
    outw(0x0005, 0x03ce);
    outw(0x0406, 0x03ce);
    outw(0x0402, 0x03c4);
    outw(0x0704, 0x03c4);
    kbcopy(_vgafontbuf, (void *)VGAFONTADR, VGAFONTSIZE);
    vgareset();

    return;
}

void
vgaputpix(int32_t pix, int32_t x, int32_t y)
{
    ;
}

/* initialise 8 consoles */
void
vgainitcon(int w, int h)
{
    struct vgacon *con = _vgacontab;
    uint8_t       *ptr = (uint8_t *)VGABUFADR;
    long           l;

    vgagetfont();
    for (l = 0 ; l < VGANCON ; l++) {
        kbzero(ptr, PAGESIZE);
        con->buf = (uint16_t *)ptr;
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
        con++;
    }
    vgacurcon = 0;
    vgamoveto(0, 0);
#if 0
    kprintf("VGA @ 0x%x - width = %d, height = %d, %d consoles\n",
            VGABUFADR, w, h, VGANCON);
#endif

    return;
}

/* output string on the current console */
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

    con = &_vgacontab[vgacurcon];
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

/* output string on a given console */
void
vgaputs2(struct vgacon *con, char *str)
{
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

    con = &_vgacontab[vgacurcon];
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

#endif /* !VBE2 */

