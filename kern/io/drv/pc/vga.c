#include <stdint.h>
#include <stddef.h>
#include <zero/cdecl.h>
#include <kern/util.h>
#include <kern/io/drv/pc/vga.h>

struct vgacon  _vgacontab[VGANCON] ALIGNED(PAGESIZE);
static long    _vgacurcon;

/* initialise 8 consoles */
void
vgainitcon(int w, int h)
{
    struct vgacon *con = _vgacontab;
    uint8_t       *ptr = (uint8_t *)VGABUFADR;
    long           l;

    for (l = 0 ; l < VGANCON ; l++) {
        bzero(ptr, PAGESIZE);
        con->buf = (uint16_t *)ptr;
        con->x = 0;
        con->y = 0;
        con->w = w;
        con->h = h;
        con->chatr = vgasetfg(0, VGAWHITE);
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
    uint16_t      *ptr;
    int            x;
    int            y;
    int            w;
    int            h;
    uint8_t        ch;
    uint8_t        atr;

    con = &_vgacontab[_vgacurcon];
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
            vgaputch3(ptr, ch, atr);
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
    uint16_t      *ptr;
    int            x;
    int            y;
    int            w;
    int            h;
    uint8_t        ch;
    uint8_t        atr;

    con = &_vgacontab[conid];
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
            vgaputch3(ptr, ch, atr);
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
    uint16_t      *ptr;

    con = &_vgacontab[_vgacurcon];
    ptr = con->buf + con->w * con->x + con->y;
    *ptr = _vgamkch(ch, con->chatr);

    return;
}

void
vgasyncscr(void)
{
    ;
}

