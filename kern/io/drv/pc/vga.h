#ifndef __KERN_IO_DRV_PC_VGA_H__
#define __KERN_IO_DRV_PC_VGA_H__

#include <stdint.h>
#include <sys/io.h>

#define VGANCON        8
#define VGABUFSIZE     (1 << VGABUFSIZELOG2)
#define VGABUFSIZELOG2 12
#define VGACHARSIZE    2
#define VGABUFADR      0x000b8000U
#define VGACONBUFSIZE  (VGANCON * VGABUFSIZE)
#define VGAFONTADR     0x000a0000
#define VGAFONTSIZE    4096
#define VGANGLYPH      256
#define VGAGLYPHH      16
#define VGAGLYPHW      8

/* text interface */

/* interface macros */
#if (VGAGFX)
#define vgasetfg(con, fg)                                               \
    ((con)->fg = (fg))
#define vgasetbg(con, bg)                                               \
    ((con)->bg = (bg))
#else
#define vgasetfg(atr, fg) ((atr) | (fg))
#define vgasetbg(atr, bg) ((atr)| ((bg) < 4))
#endif
#define vgasetblink(ch)   ((ch) | VGABLINK)
#define vgasetfgcolor(c)                                                \
    do {                                                                \
        struct vgacon *_con;                                            \
        uint16_t      _atr;                                             \
                                                                        \
        _con = &_vgacontab[_vgacurcon];                                 \
        _atr = con->chatr;                                              \
        con->chatr = vgasetfg(_atr, (c));                               \
    } while (0)
#define vgaputch(ch)                                                    \
    do {                                                                \
        struct vgacon *_con;                                            \
        uint16_t      *_buf;                                            \
        uint16_t      _atr;                                             \
                                                                        \
        _con = &_vgacontab[_vgacurcon];                                 \
        _buf = (uint8_t *)VGABUFADR + _vgacurcon * VGABUFSIZE;          \
        _atr = con->chatr;                                              \
        _buf[con->w * con->x + con->y] = _vgamkch(ch, atr);             \
    } while (0)
#define vgaputch3(ptr, ch, atr)                                         \
    (*(ptr) = _vgamkch(ch, atr))
//    ((buf)[(x) * VGACHARSIZE + (y)] = _vgamkch(ch, atr))
#define vgaputc(buf, ch)                                                \
    vgaputch(buf, ch, _vgaatrbuf[_vgaconid(buf)])
#define vgamoveto(x, y)                                                 \
    do {                                                                \
        uint16_t _ofs = (x) * (y) * sizeof(uint16_t);                   \
                                                                        \
        outb(VGACURHI, VGACRTC);                                        \
        outb(_ofs >> 8, VGACRTC + 1);                                   \
        outb(VGACURLO, VGACRTC);                                        \
        outb(_ofs & 0xff, VGACRTC + 1);                                 \
    } while (0)

/* internal macros */
#define _vgaconid(buf)                                                  \
    (((uintptr_t)(buf) - VGABUFADR) >> VGABUFSIZELOG2)
#define _vgamkch(ch, atr)                                               \
    ((uint16_t)(ch) | ((uint16_t)(atr) << 8))

#define VGACRTC       0x03b4
#define VGACURHI      0x14
#define VGACURLO      0x15

/* text attributes */
#define VGACHARMASK   0x00ff
#define VGAFGMASK     0x0f00
#define VGABGMASK     0x7000
#define VGABLINK      0x8000
#define VGACATRMASK   0xff00

/* text colors */
#define VGABLACK      0x00
#define VGABLUE       0x01
#define VGAGREEN      0x02
#define VGACYAN       0x03
#define VGARED        0x04
#define VGAMAGENTA    0x05
#define VGABROWN      0x06
#define VGAWHITE      0x07
#define VGADARKGRAY   0x08
#define VGABBLUE      0x09
#define VGABGREEN     0x0a
#define VGABCYAN      0x0b
#define VGAPINK       0x0c
#define VGABMAGENTA   0x0d
#define VGAYELLOW     0x0e
#define VGABWHITE     0x0f

/* vga [text] console structure */
struct vgacon {
#if (VGAGFX) || (VBE2)
    int32_t  fg;
    int32_t  bg;
    void    *buf;
#else
    uint16_t *buf;
#endif
    uint8_t   x;
    uint8_t   y;
    uint8_t   w;
    uint8_t   h;
#if (!VGAGFX)
    uint16_t  chatr;
#endif
    long      nbufln;   // number of buffered lines
    void     *data;     // text buffers
} PACK;

/* graphics interface */

#if 0
#define vgareset()                                                      \
    do {                                                                \
        outw(0x0302, 0x03c4);                                           \
        outw(0x1005, 0x03ce);                                           \
        outw(0x0a06, 0x03ce);                                           \
    } while (0)
#endif

void vgasyncscr(void);
void vgaputs(char *str);
void vgaputchar(int ch);

#endif /* __KERN_IO_DRV_PC_VGA_H__ */

