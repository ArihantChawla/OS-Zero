#include <kern/conf.h>

#if (PS2DRV)

/*
 * 19:49 <PeanutHorst>
 * { == curly brace   < == angle bracket   [ == bracket   ( == parentheses
 */

#include <stdint.h>
#include <sys/io.h>

#include <zero/param.h>
#include <zero/cdecl.h>

#include <kern/util.h>
//#include <kern/event.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/trap.h>
#include <kern/io/drv/pc/ps2.h>

extern void *irqvec[];

void ps2initkbd_us(void);
void ps2kbdintr(void);
void ps2mouseintr(void);

struct ps2drv {
#if 0
/* modifier keys. */
    int32_t          keytabmod[PS2KBD_NTAB];
#endif
/* single-code values. */
    int32_t          keytab1b[PS2KBD_NTAB];
/* 0xe0-prefixed values. */
    int32_t          keytabmb[PS2KBD_NTAB];
/* release values. */
    int32_t          keytabup[PS2KBD_NTAB];
    struct mousestat mousestat;
    int32_t          modmask;
};

static struct ps2drv ps2drv ALIGNED(PAGESIZE);

#define ps2readkbd(u8)                                                  \
    __asm__ ("inb %w1, %b0\n" : "=a" (u8) : "Nd" (PS2KBD_PORT))
#define ps2sendkbd(u8)                                                  \
    __asm__ ("outb %b0, %w1\n" : : "a" (u8), "Nd" (PS2KBD_PORT))
#define ps2readmouse(u8)                                                \
    __asm__("inb %w1, %b0" : "=a" (u8) : "i" (PS2MOUSE_INPORT))

#if 0
#define ps2setkeymod(name)                                              \
    (keytabmod[name] = (name##_FLAG))
#endif
#define ismodkey(val)                                                   \
    (((val) & 0x80000000) && ((val) & 0xfffffff0) == 0xfffffff0)
#if 0
#define ps2setkeymod(name)                                              \
    (keytabmod[name] = (1 << (-(name##_SYM))))
#endif
#define ps2setkeycode(name)                                             \
    (((((name) >> 8) & 0xff) == PS2KBD_UP_BYTE)                         \
     ? (ps2drv.keytabup[name >> 16] = name##_SYM | PS2KBD_UP_BIT)       \
     : ((((name) & 0xff) == PS2KBD_PREFIX_BYTE)                         \
        ? (ps2drv.keytabmb[name >> 8] = name##_SYM,                     \
           ps2drv.keytabup[name >> 8] = name##_SYM | PS2KBD_UP_BIT)     \
        : (ps2drv.keytab1b[name] = name##_SYM,                          \
           ps2drv.keytabup[name] = name##_SYM | PS2KBD_UP_BIT)))

void
kbdinit(void)
{
    uint8_t u8;

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
    ps2initkbd_us();
    kprintf("PS/2 keyboard with US keymap initialized\n");
    irqvec[IRQKBD] = ps2kbdintr;
    kprintf("PS/2 keyboard interrupt enabled\n");

    return;
}

void
ps2initkbd_us(void)
{
    /* modifiers. */
    ps2setkeycode(PS2KBD_LEFTCTRL);
    ps2setkeycode(PS2KBD_LEFTSHIFT);
    ps2setkeycode(PS2KBD_RIGHTSHIFT);
    ps2setkeycode(PS2KBD_LEFTALT);
    ps2setkeycode(PS2KBD_RIGHTALT);
    ps2setkeycode(PS2KBD_CAPSLOCK);
    ps2setkeycode(PS2KBD_NUMLOCK);
    ps2setkeycode(PS2KBD_SCROLLLOCK);

    /* single-byte keys. */

    ps2setkeycode(PS2KBD_ESC);
    ps2setkeycode(PS2KBD_1);
    ps2setkeycode(PS2KBD_2);
    ps2setkeycode(PS2KBD_3);
    ps2setkeycode(PS2KBD_4);
    ps2setkeycode(PS2KBD_5);
    ps2setkeycode(PS2KBD_6);
    ps2setkeycode(PS2KBD_7);
    ps2setkeycode(PS2KBD_8);
    ps2setkeycode(PS2KBD_9);
    ps2setkeycode(PS2KBD_0);
    ps2setkeycode(PS2KBD_MINUS);
    ps2setkeycode(PS2KBD_PLUS);
    ps2setkeycode(PS2KBD_BACKSPACE);

    ps2setkeycode(PS2KBD_TAB);
    ps2setkeycode(PS2KBD_q);
    ps2setkeycode(PS2KBD_w);
    ps2setkeycode(PS2KBD_e);
    ps2setkeycode(PS2KBD_r);
    ps2setkeycode(PS2KBD_t);
    ps2setkeycode(PS2KBD_y);
    ps2setkeycode(PS2KBD_u);
    ps2setkeycode(PS2KBD_i);
    ps2setkeycode(PS2KBD_o);
    ps2setkeycode(PS2KBD_p);
    ps2setkeycode(PS2KBD_OPENBRACKET);
    ps2setkeycode(PS2KBD_CLOSEBRACKET);

    ps2setkeycode(PS2KBD_ENTER);

    ps2setkeycode(PS2KBD_LEFTCTRL);

    ps2setkeycode(PS2KBD_a);
    ps2setkeycode(PS2KBD_s);
    ps2setkeycode(PS2KBD_d);
    ps2setkeycode(PS2KBD_f);
    ps2setkeycode(PS2KBD_g);
    ps2setkeycode(PS2KBD_h);
    ps2setkeycode(PS2KBD_i);
    ps2setkeycode(PS2KBD_j);
    ps2setkeycode(PS2KBD_k);
    ps2setkeycode(PS2KBD_l);
    ps2setkeycode(PS2KBD_SEMICOLON);
    ps2setkeycode(PS2KBD_QUOTE);

    ps2setkeycode(PS2KBD_BACKQUOTE);

    ps2setkeycode(PS2KBD_LEFTSHIFT);

    ps2setkeycode(PS2KBD_BACKSLASH);

    ps2setkeycode(PS2KBD_z);
    ps2setkeycode(PS2KBD_x);
    ps2setkeycode(PS2KBD_c);
    ps2setkeycode(PS2KBD_v);
    ps2setkeycode(PS2KBD_b);
    ps2setkeycode(PS2KBD_n);
    ps2setkeycode(PS2KBD_m);
    ps2setkeycode(PS2KBD_COMMA);
    ps2setkeycode(PS2KBD_DOT);
    ps2setkeycode(PS2KBD_SLASH);

    ps2setkeycode(PS2KBD_RIGHTSHIFT);

    ps2setkeycode(PS2KBD_KEYPADASTERISK);

    ps2setkeycode(PS2KBD_SPACE);

    ps2setkeycode(PS2KBD_CAPSLOCK);

    ps2setkeycode(PS2KBD_F1);
    ps2setkeycode(PS2KBD_F2);
    ps2setkeycode(PS2KBD_F3);
    ps2setkeycode(PS2KBD_F4);
    ps2setkeycode(PS2KBD_F5);
    ps2setkeycode(PS2KBD_F6);
    ps2setkeycode(PS2KBD_F7);
    ps2setkeycode(PS2KBD_F8);
    ps2setkeycode(PS2KBD_F9);
    ps2setkeycode(PS2KBD_F10);

    ps2setkeycode(PS2KBD_NUMLOCK);
    ps2setkeycode(PS2KBD_SCROLLLOCK);

    ps2setkeycode(PS2KBD_F11);
    ps2setkeycode(PS2KBD_F12);

    ps2setkeycode(PS2KBD_KEYPAD7);
    ps2setkeycode(PS2KBD_KEYPAD8);
    ps2setkeycode(PS2KBD_KEYPAD9);

    ps2setkeycode(PS2KBD_KEYPADMINUS2);

    ps2setkeycode(PS2KBD_KEYPAD4);
    ps2setkeycode(PS2KBD_KEYPAD5);
    ps2setkeycode(PS2KBD_KEYPAD6);

    ps2setkeycode(PS2KBD_KEYPADPLUS);

    ps2setkeycode(PS2KBD_KEYPADEND);
    ps2setkeycode(PS2KBD_KEYPADDOWN);
    ps2setkeycode(PS2KBD_KEYPADPGDN);

    ps2setkeycode(PS2KBD_KEYPADINS);
    ps2setkeycode(PS2KBD_KEYPADDEL);

    ps2setkeycode(PS2KBD_SYSRQ);

    /* dual-byte sequences. */

    ps2setkeycode(PS2KBD_KEYPADENTER);
    ps2setkeycode(PS2KBD_RIGHTCTRL);
    ps2setkeycode(PS2KBD_FAKELEFTSHIFT);
    ps2setkeycode(PS2KBD_KEYPADMINUS3);
    ps2setkeycode(PS2KBD_FAKERIGHTSHIFT);
    ps2setkeycode(PS2KBD_CTRLPRINTSCREEN);
    ps2setkeycode(PS2KBD_RIGHTALT);
    ps2setkeycode(PS2KBD_CTRLBREAK);
    ps2setkeycode(PS2KBD_HOME);
    ps2setkeycode(PS2KBD_UP);
    ps2setkeycode(PS2KBD_PGUP);
    ps2setkeycode(PS2KBD_LEFT);
    ps2setkeycode(PS2KBD_RIGHT);
    ps2setkeycode(PS2KBD_END);
    ps2setkeycode(PS2KBD_DOWN);
    ps2setkeycode(PS2KBD_PGDN);
    ps2setkeycode(PS2KBD_INS);
    ps2setkeycode(PS2KBD_DEL);

    /* acpi codes. */
    ps2setkeycode(PS2KBD_POWER);
    ps2setkeycode(PS2KBD_SLEEP);
    ps2setkeycode(PS2KBD_WAKE);
    ps2setkeycode(PS2KBD_POWERUP);
    ps2setkeycode(PS2KBD_SLEEPUP);
    ps2setkeycode(PS2KBD_WAKEUP);

    return;
}

/* keyboard interrupt handler. */
void
ps2kbdintr(void)
{
    int32_t isup = 0;
    int32_t val;
    uint8_t u8;
    
    val = 0;
    ps2readkbd(u8);
    if (u8 == PS2KBD_PAUSE_BYTE1) {
        /* pause/break. */
        ps2readkbd(u8); /* 0x1d */
        ps2readkbd(u8); /* 0x45 */
        ps2readkbd(u8); /* 0xe1 */
        ps2readkbd(u8); /* 0x9d */
        ps2readkbd(u8); /* 0xc5 */
        u8 &= PS2KBD_VAL_MSK;
        val = ps2drv.keytab1b[u8];
    } else if (u8 != PS2KBD_PREFIX_BYTE) {
        /* single-byte value. */
        if (u8 & PS2KBD_UP_BIT) {
            /* release. */
            isup = 1;
            u8 &= ~PS2KBD_UP_BIT;
            val = ps2drv.keytabup[u8];
        } else {
            val = ps2drv.keytab1b[u8];
        }
    } else {
        /* 0xe0-prefixed. */
        ps2readkbd(u8);
        if (u8 == PS2KBD_PRINT_BYTE2 || u8 == PS2KBD_CTRLPAUSE_BYTE2) {
            /* print screen or ctrl-pause. */
            ps2readkbd(u8); /* 0xe0 */
            ps2readkbd(u8); /* 0x37 (prtsc) or 0xc6 (ctrl-pause) */
            val &= PS2KBD_VAL_MSK;
            val = ps2drv.keytabmb[u8];
        } else if (u8 == PS2KBD_UP_BYTE) {
            ps2readkbd(u8);
            val = ps2drv.keytabup[u8];
        } else {
            if (u8 & PS2KBD_UP_BIT) {
                isup = 1;
                u8 &= ~PS2KBD_UP_BIT;
                val = ps2drv.keytabup[u8];
            } else {
                val = ps2drv.keytabmb[u8];
            }
        }
    }
    if (ismodkey(val)) {
        if (isup) {
            ps2drv.modmask &= ~(1 << (-val));
        } else {
            ps2drv.modmask |= 1 << (-val);
        }
    }

    return;
}

void
mouseinit(void)
{
    irqvec[IRQMOUSE] = ps2mouseintr;
    kprintf("PS/2 mouse interrupt enabled\n");

    return;
}

void
ps2mouseintr(void)
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

    ps2readmouse(msk);
    ps2readmouse(u8);
    xmov = u8;
    ps2readmouse(u8);
    ymov = u8;

    val = ps2drv.mousestat.flags;
    zmov = 0;
    val &= PS2MOUSE_WHEELMSK;                     /* scroll-wheel?. */
    stat = msk & PS2MOUSE_3BTNMSK;                /* button 1, 2 & 3 states. */
    if (val) {
        /* mouse with scroll-wheel, extra (4th) data byte. */
        ps2readmouse(u8);
        xtra = u8;
        val &= PS2MOUSE_WHEEL5BTN;                /* 5-button?. */
        zmov = xtra & PS2MOUSE_ZMSK;              /* z-axis movement. */
        tmp = xtra & PS2MOUSE_ZSIGN;              /* extract sign bit. */
        if (val) {
            stat |= (xtra >> 1) & PS2MOUSE_XBTNMSK; /* button 4 & 5 states. */
        }
        if (tmp) {
            zmov = -zmov;
        }
    }
    ps2drv.mousestat.state = stat;

    shift = ps2drv.mousestat.shift;               /* scale (speed) value. */

    val = ps2drv.mousestat.x;
    tmp = msk & PS2MOUSE_XOVERFLOW;
    if (tmp) {
        xmov = 0xff;
    } else if (shift > 0) {
        xmov <<= shift;
    } else {
        xmov >>= shift;
    }
    tmp = msk & PS2MOUSE_XSIGN;
//    xmov |= tmp << 27; /* sign. */
    if (tmp) {
        xmov = -xmov;
    }
    if (xmov < 0) {
        ps2drv.mousestat.x = (val < -xmov) ? 0 : (val + xmov);
    } else {
        tmp = ps2drv.mousestat.xmax;
        ps2drv.mousestat.x = (val < tmp - val) ? (val + xmov) : tmp;
    }

    val = ps2drv.mousestat.y;
    tmp = msk & PS2MOUSE_YOVERFLOW;
    if (tmp) {
        ymov = 0xff;
    } else if (shift > 0) {
        ymov <<= shift;
    } else {
        ymov >>= shift;
    }
    tmp = msk & PS2MOUSE_YSIGN;
//    ymov |= tmp << 26; /* sign. */
    if (tmp) {
        ymov = -ymov;
    }
    if (ymov < 0) {
        ps2drv.mousestat.y = (val < -ymov) ? 0 : (val + ymov);
    } else {
        tmp = ps2drv.mousestat.ymax;
        ps2drv.mousestat.y = (val < tmp - val) ? (val + ymov) : tmp;
    }

    if (zmov) {
        val = ps2drv.mousestat.z;
        if (zmov < 0) {
            ps2drv.mousestat.z = (val < -zmov) ? 0 : (val + zmov);
        } else {
            tmp = ps2drv.mousestat.zmax;
            ps2drv.mousestat.z = (val < tmp - val) ? (val + zmov) : tmp;
        }
    }

    return;
}

void
ps2init(void)
{
    kbdinit();
    mouseinit();
}

#endif /* PS2DRV */

