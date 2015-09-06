#include <kern/conf.h>

#if (PS2DRV)

/*
 * 19:49 <PeanutHorst>
 * { == curly brace   < == angle bracket   [ == bracket   ( == parentheses
 */

#include <stddef.h>
#include <stdint.h>
#include <limits.h>
#include <sys/io.h>
#include <sys/zero/ps2.h>

#include <zero/param.h>
#include <zero/cdecl.h>
#include <zero/mtx.h>
//#define VAL(x)        (x)
#define NOP(x)        ((void)0)
#define NOP3(x, y, z) ((void)0)
#define MALLOC(x)     ((void *)0)
#define FREE          NOP
#define MEMCPY        NOP3
#define RING_ITEM     uint64_t
#define RING_INVAL    UINT64_C(0)
#include <zero/ring.h>

#include <kern/util.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/trap.h>
#include <kern/io/drv/pc/ps2.h>

extern void *irqvec[];

//void ps2initkbd_us(void);
void ps2kbdintr(void);
void ps2mouseintr(void);

unsigned char kbdbuf[PAGESIZE / CHAR_BIT] ALIGNED(PAGESIZE);
static struct ps2drv ps2drv;

#define ps2readkbd(u8)                                                  \
    __asm__ ("inb %w1, %b0\n" : "=a" (u8) : "i" (PS2KBD_PORT))
#define ps2sendkbd(u8)                                                  \
    __asm__ ("outb %b0, %w1\n" : : "a" (u8), "i" (PS2KBD_PORT))
#define ps2readmouse(u8)                                                \
    __asm__("inb %w1, %b0" : "=a" (u8) : "i" (PS2MOUSE_INPORT))

void
ps2initkbd(void)
{
    uint8_t u8;

    ringinit(&kbdbuf,
             kbdbuf + offsetof(struct ringbuf, data),
             (PAGESIZE - offsetof(struct ringbuf, data)) / sizeof(RING_ITEM));
    ps2drv.buf = &kbdbuf;
    /* enable keyboard */
    ps2sendkbd(PS2KBD_ENABLE);
    do {
        ps2readkbd(u8);
    } while (u8 != PS2KBD_ACK);
    /* choose scancode set 1 */
    ps2sendkbd(PS2KBD_SETSCAN);
    do {
        ps2readkbd(u8);
    } while (u8 != PS2KBD_ACK);
    ps2sendkbd(0x01);
    do {
        ps2readkbd(u8);
    } while (u8 != PS2KBD_ACK);
    irqvec[IRQKBD] = ps2kbdintr;
    kprintf("PS/2 keyboard interrupt enabled\n");

    return;
}

void
ps2kbdflush(uint64_t keycode, int32_t keyval)
{
    ;
}

void
ps2kbdaddkey(uint64_t keycode)
{
    mtxlk(&ps2drv.buf->lk);
    ringput(ps2drv.buf, keycode);
    mtxunlk(&ps2drv.buf->lk);
}

/* keyboard interrupt handler. */
void
ps2kbdintr(void)
{
    uint64_t keycode;
    uint8_t  u8;
    
    ps2readkbd(u8);
    keycode = u8;
    if (u8 == PS2KBD_PAUSE_BYTE1) {
        /* pause/break. */
        ps2readkbd(u8); /* 0x1d */
        keycode |= UINT64_C(0x1d) << 8;
        ps2readkbd(u8); /* 0x45 */
        keycode |= UINT64_C(0x45) << 16;
        ps2readkbd(u8); /* 0xe1 */
        keycode |= UINT64_C(0xe1) << 24;
        ps2readkbd(u8); /* 0x9d */
        keycode |= UINT64_C(0x9d) << 32;
        ps2readkbd(u8); /* 0xc5 */
        keycode |= UINT64_C(0xc5) << 40;
    } else if (u8 & PS2KBD_PREFIX_BYTE) {
        /* 0xe0-prefixed. */
        ps2readkbd(u8); /* 0xe0 */
        keycode |= UINT64_C(0xe0) << 8;
        if (u8 == PS2KBD_PRINT_BYTE2
            || u8 == PS2KBD_CTRLPAUSE_BYTE2
            || u8 == PS2KBD_UP_BYTE) {
            ps2readkbd(u8);
            keycode |= (uint64_t)u8 << 16;
        }
    }
    ps2kbdaddkey(keycode);

    return;
}

void
ps2initmouse(void)
{
    irqvec[IRQMOUSE] = ps2mouseintr;
    kprintf("PS/2 mouse interrupt enabled\n");

    return;
}

void
ps2mouseintr(void)
{
    int32_t val;
    int32_t xmov;
    int32_t ymov;
    int32_t zmov;
    int32_t xtra;
    int32_t shift;
    int32_t tmp;
    uint8_t mask;
    uint8_t state;
    uint8_t u8;

    ps2readmouse(mask);
    ps2readmouse(u8);
    xmov = u8;
    ps2readmouse(u8);
    ymov = u8;
    val = ps2drv.mousestate.flg;
    zmov = 0;
    val &= PS2MOUSE_WHEELMASK;          /* scroll-wheel?. */
    state = mask & PS2MOUSE_3BTNMASK;   /* button 1, 2 & 3 states. */
    if (val) {
        /* mouse with scroll-wheel, extra (4th) data byte. */
        ps2readmouse(u8);
        xtra = u8;
        val &= PS2MOUSE_WHEEL5BTN;      /* 5-button?. */
        zmov = xtra & PS2MOUSE_ZMASK;   /* z-axis movement. */
        tmp = xtra & PS2MOUSE_ZSIGN;    /* extract sign bit. */
        if (val) {
            state |= (xtra >> 1) & PS2MOUSE_XBTNMASK; /* button 4 & 5 states. */
        }
        if (tmp) {
            zmov = -zmov;
        }
    }
    ps2drv.mousestate.state = state;
    shift = ps2drv.mousestate.shift;    /* scale (speed) value. */
    val = ps2drv.mousestate.x;
    tmp = mask & PS2MOUSE_XOVERFLOW;
    if (tmp) {
        xmov = 0xff;
    } else if (shift > 0) {
        xmov <<= shift;
    } else {
        xmov >>= shift;
    }
    tmp = mask & PS2MOUSE_XSIGN;
    if (tmp) {
        xmov = -xmov;
    }
    if (xmov < 0) {
        ps2drv.mousestate.x = (val < -xmov) ? 0 : (val + xmov);
    } else {
        tmp = ps2drv.mousestate.xmax;
        ps2drv.mousestate.x = (val < tmp - val) ? (val + xmov) : tmp;
    }
    val = ps2drv.mousestate.y;
    tmp = mask & PS2MOUSE_YOVERFLOW;
    if (tmp) {
        ymov = 0xff;
    } else if (shift > 0) {
        ymov <<= shift;
    } else {
        ymov >>= shift;
    }
    tmp = mask & PS2MOUSE_YSIGN;
    if (tmp) {
        ymov = -ymov;
    }
    if (ymov < 0) {
        ps2drv.mousestate.y = (val < -ymov) ? 0 : (val + ymov);
    } else {
        tmp = ps2drv.mousestate.ymax;
        ps2drv.mousestate.y = (val < tmp - val) ? (val + ymov) : tmp;
    }
    if (zmov) {
        val = ps2drv.mousestate.z;
        if (zmov < 0) {
            ps2drv.mousestate.z = (val < -zmov) ? 0 : (val + zmov);
        } else {
            tmp = ps2drv.mousestate.zmax;
            ps2drv.mousestate.z = (val < tmp - val) ? (val + zmov) : tmp;
        }
    }

    return;
}

void
ps2init(void)
{
    ps2initkbd();
    ps2initmouse();
}

#endif /* PS2DRV */

