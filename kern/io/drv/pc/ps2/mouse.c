#include <kern/conf.h>

#if (PS2DRV)

#include <stdint.h>
#include <kern/util.h>
#include <zero/cdecl.h>
#include <kern/ia32/trap.h>

extern void *irqvec[];

void mouseint(void);

#define MOUSE_INPORT    0x60

/* state. */
#define MOUSE_LEFTBTN   0x01 /* left button flag. */
#define MOUSE_RIGHTBTN  0x02 /* right button flag. */
#define MOUSE_MIDDLEBTN 0x04 /* middle button flag. */
#define MOUSE_3BTNMSK   0x07 /* 3-button mask. */
#define MOUSE_BTN4      0x08 /* button 4 flag. */
#define MOUSE_BTN5      0x10 /* button 5 flag. */
#define MOUSE_XBTNMSK   0x18 /* extra button (4 & 5) mask. */
/* other data. */
#define MOUSE_XSIGN     0x10 /* x-movement sign. */
#define MOUSE_YSIGN     0x20 /* y-movement sign. */
#define MOUSE_XOVERFLOW 0x40 /* x-movement overflow. */
#define MOUSE_YOVERFLOW 0x80 /* y-movement overflow. */
/* extra byte. */
#define MOUSE_ZSIGN     0x08 /* z-movement sign. */
#define MOUSE_ZMSK      0x07 /* z-movement mask. */
#define MOUSE_5BTNMSK   0x30 /* extra button (4 & 5) mask. */
/* flags. */
#define MOUSE_WHEEL      0x01 /* scroll wheel flag. */
#define MOUSE_WHEEL5BTN 0x02 /* 5-button flag. */
#define MOUSE_WHEELMSK  0x03 /* intellimouse mask. */
struct mousestat {
    uint16_t flags;
    uint16_t state;
    uint32_t x;
    uint32_t y;
    uint32_t z;
    uint32_t xmax;
    uint32_t ymax;
    uint32_t zmax;
    int32_t shift;
} PACK();

static struct mousestat _mousestat ALIGNED(CLSIZE);

#define mouseread(u8)                                                   \
    __asm__("inb %w1, %b0" : "=a" (u8) : "i" (MOUSE_INPORT))

void
mouseinit(void)
{
    irqvec[IRQMOUSE] = mouseint;
    kprintf("PS/2 mouse interrupt enabled\n");

    return;
}

void
mouseint(void)
{
    uint32_t val;
    int32_t  xmov;
    int32_t  ymov;
    int32_t  zmov;
    int32_t  xtra;
    int32_t  shift;
    int32_t  tmp;
    uint8_t  msk;
    uint8_t  stat;
    uint8_t  u8;

    mouseread(msk);
    mouseread(u8);
    xmov = u8;
    mouseread(u8);
    ymov = u8;

    val = _mousestat.flags;
    zmov = 0;
    val &= MOUSE_WHEELMSK;                        /* scroll-wheel?. */
    stat = msk & MOUSE_3BTNMSK;                   /* button 1, 2 & 3 states. */
    if (val) {
        /* mouse with scroll-wheel, extra (4th) data byte. */
        mouseread(u8);
        xtra = u8;
        val &= MOUSE_WHEEL5BTN;                   /* 5-button?. */
        zmov = xtra & MOUSE_ZMSK;                 /* z-axis movement. */
        tmp = xtra & MOUSE_ZSIGN;                 /* extract sign bit. */
        if (val) {
            stat |= (xtra >> 1) & MOUSE_XBTNMSK;  /* button 4 & 5 states. */
        }
        if (tmp) {
            zmov = -zmov;
        }
    }
    _mousestat.state = stat;

    shift = _mousestat.shift;                     /* scale (speed) value. */

    val = _mousestat.x;
    tmp = msk & MOUSE_XOVERFLOW;
    if (tmp) {
        xmov = 0xff;
    } else if (shift > 0) {
        xmov <<= shift;
    } else {
        xmov >>= shift;
    }
    tmp = msk & MOUSE_XSIGN;
//    xmov |= tmp << 27; /* sign. */
    if (tmp) {
        xmov = -xmov;
    }
    if (xmov < 0) {
        _mousestat.x = (val < -xmov) ? 0 : (val + xmov);
    } else {
        tmp = _mousestat.xmax;
        _mousestat.x = (val < tmp - val) ? (val + xmov) : tmp;
    }

    val = _mousestat.y;
    tmp = msk & MOUSE_YOVERFLOW;
    if (tmp) {
        ymov = 0xff;
    } else if (shift > 0) {
        ymov <<= shift;
    } else {
        ymov >>= shift;
    }
    tmp = msk & MOUSE_YSIGN;
//    ymov |= tmp << 26; /* sign. */
    if (tmp) {
        ymov = -ymov;
    }
    if (ymov < 0) {
        _mousestat.y = (val < -ymov) ? 0 : (val + ymov);
    } else {
        tmp = _mousestat.ymax;
        _mousestat.y = (val < tmp - val) ? (val + ymov) : tmp;
    }

    if (zmov) {
        val = _mousestat.z;
        if (zmov < 0) {
            _mousestat.z = (val < -zmov) ? 0 : (val + zmov);
        } else {
            tmp = _mousestat.zmax;
            _mousestat.z = (val < tmp - val) ? (val + zmov) : tmp;
        }
    }

    return;
}

#endif /* PS2DRV */

