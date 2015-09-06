#ifndef __KERN_IO_DRV_PC_PS2_H__
#define __KERN_IO_DRV_PC_PS2_H__

#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/ring.h>

#define PS2KBD_PORT                 0x60

/* keyboard commands */
#define PS2KBD_ENABLE               0xf4
#define PS2KBD_SETSCAN              0xf0

/* MOUSE */

#define PS2MOUSE_INPORT    0x60

/* state. */
#define PS2MOUSE_LEFTBTN   0x01 /* left button flag. */
#define PS2MOUSE_RIGHTBTN  0x02 /* right button flag. */
#define PS2MOUSE_MIDDLEBTN 0x04 /* middle button flag. */
#define PS2MOUSE_3BTNMASK  0x07 /* 3-button mask. */
#define PS2MOUSE_BTN4      0x08 /* button 4 flag. */
#define PS2MOUSE_BTN5      0x10 /* button 5 flag. */
#define PS2MOUSE_XBTNMASK  0x18 /* extra button (4 & 5) mask. */
/* other data. */
#define PS2MOUSE_XSIGN     0x10 /* x-movement sign. */
#define PS2MOUSE_YSIGN     0x20 /* y-movement sign. */
#define PS2MOUSE_XOVERFLOW 0x40 /* x-movement overflow. */
#define PS2MOUSE_YOVERFLOW 0x80 /* y-movement overflow. */
/* extra byte. */
#define PS2MOUSE_ZSIGN     0x08 /* z-movement sign. */
#define PS2MOUSE_ZMASK     0x07 /* z-movement mask. */
#define PS2MOUSE_5BTNMASK  0x30 /* extra button (4 & 5) mask. */
/* flags. */
#define PS2MOUSE_WHEEL     0x01 /* scroll wheel flag. */
#define PS2MOUSE_WHEEL5BTN 0x02 /* 5-button flag. */
#define PS2MOUSE_WHEELMASK 0x03 /* intellimouse mask. */
struct ps2mousestate {
    uint16_t flg;
    uint16_t state;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t xmax;
    uint32_t ymax;
    uint32_t zmax;
    int32_t shift;
};

#if 0
struct ps2drv {
#if 0
    uint64_t          keybuf[PS2KBD_NBUFKEY];
    int32_t           keyvalbuf[PS2KBD_NBUFKEY];
#endif
    volatile long     keylk;
    uint64_t         *curkey;
    uint64_t         *lastkey;
    int32_t          *curkeyval;
    int32_t          *lastkeyval;
#if 0
/* modifier keys. */
    int32_t           keytabmod[PS2KBD_NTAB];
#endif
/* single-code values. */
    int32_t           keytab1b[PS2KBD_NTAB] ALIGNED(PAGESIZE);
/* 0xe0-prefixed values. */
    int32_t           keytabmb[PS2KBD_NTAB];
/* release values. */
    int32_t           keytabup[PS2KBD_NTAB];
    struct mousestate mousestate;
    int32_t           modmask;
} ALIGNED(PAGESIZE);
#endif

struct ps2drv {
    struct ringbuf       *buf;
    struct ps2mousestate  mousestate;
};

#endif /* __KERN_IO_DRV_PC_PS2_H__ */

