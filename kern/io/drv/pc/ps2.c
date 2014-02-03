#include <kern/conf.h>

#if (PS2DRV)

/*
 * 19:49 <PeanutHorst>
 * { == curly brace   < == angle bracket   [ == bracket   ( == parentheses
 */

#include <stdint.h>
#include <sys/io.h>
#include <kern/util.h>

#include <zero/param.h>
#include <zero/cdecl.h>

//#include <kern/event.h>
#include <kern/unit/x86/trap.h>
#include <kern/unit/x86/trap.h>
#include <kern/io/drv/pc/ps2.h>

extern void *irqvec[];

void kbdinit_us(void);
void kbdintr(void);
void mouseintr(void);

#if 0
/* modifier keys. */
static int32_t keytabmod[KBD_NTAB];
#endif
/* single-code values. */
static int32_t keytab1b[KBD_NTAB] ALIGNED(PAGESIZE);
/* 0xe0-prefixed values. */
static int32_t keytabmb[KBD_NTAB];
/* release values. */
static int32_t keytabup[KBD_NTAB];
static struct mousestat _mousestat;
static int32_t modmask;

#define kbdread(u8)                                                     \
    __asm__ ("inb %w1, %b0\n" : "=a" (u8) : "Nd" (KBD_PORT))
#define kbdsend(u8)                                                     \
    __asm__ ("outb %b0, %w1\n" : : "a" (u8), "Nd" (KBD_PORT))
#define mouseread(u8)                                                   \
    __asm__("inb %w1, %b0" : "=a" (u8) : "i" (MOUSE_INPORT))

#if 0
#define setkeymod(name)                                                 \
    (keytabmod[name] = (name##_FLAG))
#endif
#define ismodkey(val)                                                   \
    (((val) & 0x80000000) && ((val) & 0xfffffff0) == 0xfffffff0)
#if 0
#define setkeymod(name)                                                 \
    (keytabmod[name] = (1 << (-(name##_SYM))))
#endif
#define setkeycode(name)                                    \
    (((((name) >> 8) & 0xff) == KBD_UP_BYTE)                \
     ? (keytabup[name >> 16] = name##_SYM | KBD_UP_BIT)     \
     : ((((name) & 0xff) == KBD_PREFIX_BYTE)                \
        ? (keytabmb[name >> 8] = name##_SYM,                \
           keytabup[name >> 8] = name##_SYM | KBD_UP_BIT)   \
        : (keytab1b[name] = name##_SYM,                     \
           keytabup[name] = name##_SYM | KBD_UP_BIT)))

void
kbdinit(void)
{
    uint8_t u8;

    /* enable keyboard */
    kbdsend(KBD_ENABLE);
    do {
        kbdread(u8);
    } while (u8 != KBD_ACK);
    /* choose scancode set 1 */
    kbdsend(KBD_SETSCAN);
    do {
        kbdread(u8);
    } while (u8 != KBD_ACK);
    kbdsend(0x01);
    do {
        kbdread(u8);
    } while (u8 != KBD_ACK);
    kbdinit_us();
    kprintf("PS/2 keyboard with US keymap initialized\n");
    irqvec[IRQKBD] = kbdintr;
    kprintf("PS/2 keyboard interrupt enabled\n");

    return;
}

void
kbdinit_us(void)
{
    /* modifiers. */
    setkeycode(KBD_LEFTCTRL);
    setkeycode(KBD_LEFTSHIFT);
    setkeycode(KBD_RIGHTSHIFT);
    setkeycode(KBD_LEFTALT);
    setkeycode(KBD_RIGHTALT);
    setkeycode(KBD_CAPSLOCK);
    setkeycode(KBD_NUMLOCK);
    setkeycode(KBD_SCROLLLOCK);

    /* single-byte keys. */

    setkeycode(KBD_ESC);
    setkeycode(KBD_1);
    setkeycode(KBD_2);
    setkeycode(KBD_3);
    setkeycode(KBD_4);
    setkeycode(KBD_5);
    setkeycode(KBD_6);
    setkeycode(KBD_7);
    setkeycode(KBD_8);
    setkeycode(KBD_9);
    setkeycode(KBD_0);
    setkeycode(KBD_MINUS);
    setkeycode(KBD_PLUS);
    setkeycode(KBD_BACKSPACE);

    setkeycode(KBD_TAB);
    setkeycode(KBD_q);
    setkeycode(KBD_w);
    setkeycode(KBD_e);
    setkeycode(KBD_r);
    setkeycode(KBD_t);
    setkeycode(KBD_y);
    setkeycode(KBD_u);
    setkeycode(KBD_i);
    setkeycode(KBD_o);
    setkeycode(KBD_p);
    setkeycode(KBD_OPENBRACKET);
    setkeycode(KBD_CLOSEBRACKET);

    setkeycode(KBD_ENTER);

    setkeycode(KBD_LEFTCTRL);

    setkeycode(KBD_a);
    setkeycode(KBD_s);
    setkeycode(KBD_d);
    setkeycode(KBD_f);
    setkeycode(KBD_g);
    setkeycode(KBD_h);
    setkeycode(KBD_i);
    setkeycode(KBD_j);
    setkeycode(KBD_k);
    setkeycode(KBD_l);
    setkeycode(KBD_SEMICOLON);
    setkeycode(KBD_QUOTE);

    setkeycode(KBD_BACKQUOTE);

    setkeycode(KBD_LEFTSHIFT);

    setkeycode(KBD_BACKSLASH);

    setkeycode(KBD_z);
    setkeycode(KBD_x);
    setkeycode(KBD_c);
    setkeycode(KBD_v);
    setkeycode(KBD_b);
    setkeycode(KBD_n);
    setkeycode(KBD_m);
    setkeycode(KBD_COMMA);
    setkeycode(KBD_DOT);
    setkeycode(KBD_SLASH);

    setkeycode(KBD_RIGHTSHIFT);

    setkeycode(KBD_KEYPADASTERISK);

    setkeycode(KBD_SPACE);

    setkeycode(KBD_CAPSLOCK);

    setkeycode(KBD_F1);
    setkeycode(KBD_F2);
    setkeycode(KBD_F3);
    setkeycode(KBD_F4);
    setkeycode(KBD_F5);
    setkeycode(KBD_F6);
    setkeycode(KBD_F7);
    setkeycode(KBD_F8);
    setkeycode(KBD_F9);
    setkeycode(KBD_F10);

    setkeycode(KBD_NUMLOCK);
    setkeycode(KBD_SCROLLLOCK);

    setkeycode(KBD_F11);
    setkeycode(KBD_F12);

    setkeycode(KBD_KEYPAD7);
    setkeycode(KBD_KEYPAD8);
    setkeycode(KBD_KEYPAD9);

    setkeycode(KBD_KEYPADMINUS2);

    setkeycode(KBD_KEYPAD4);
    setkeycode(KBD_KEYPAD5);
    setkeycode(KBD_KEYPAD6);

    setkeycode(KBD_KEYPADPLUS);

    setkeycode(KBD_KEYPADEND);
    setkeycode(KBD_KEYPADDOWN);
    setkeycode(KBD_KEYPADPGDN);

    setkeycode(KBD_KEYPADINS);
    setkeycode(KBD_KEYPADDEL);

    setkeycode(KBD_SYSRQ);

    /* dual-byte sequences. */

    setkeycode(KBD_KEYPADENTER);
    setkeycode(KBD_RIGHTCTRL);
    setkeycode(KBD_FAKELEFTSHIFT);
    setkeycode(KBD_KEYPADMINUS3);
    setkeycode(KBD_FAKERIGHTSHIFT);
    setkeycode(KBD_CTRLPRINTSCREEN);
    setkeycode(KBD_RIGHTALT);
    setkeycode(KBD_CTRLBREAK);
    setkeycode(KBD_HOME);
    setkeycode(KBD_UP);
    setkeycode(KBD_PGUP);
    setkeycode(KBD_LEFT);
    setkeycode(KBD_RIGHT);
    setkeycode(KBD_END);
    setkeycode(KBD_DOWN);
    setkeycode(KBD_PGDN);
    setkeycode(KBD_INS);
    setkeycode(KBD_DEL);

    /* acpi codes. */
    setkeycode(KBD_POWER);
    setkeycode(KBD_SLEEP);
    setkeycode(KBD_WAKE);
    setkeycode(KBD_POWERUP);
    setkeycode(KBD_SLEEPUP);
    setkeycode(KBD_WAKEUP);

    return;
}

/* keyboard interrupt handler. */
void
kbdintr(void)
{
    int32_t isup = 0;
    int32_t val;
    uint8_t u8;
    
    val = 0;
    kbdread(u8);
    if (u8 == KBD_PAUSE_BYTE1) {
        /* pause/break. */
        kbdread(u8); /* 0x1d */
        kbdread(u8); /* 0x45 */
        kbdread(u8); /* 0xe1 */
        kbdread(u8); /* 0x9d */
        kbdread(u8); /* 0xc5 */
        u8 &= KBD_VAL_MSK;
        val = keytab1b[u8];
    } else if (u8 != KBD_PREFIX_BYTE) {
        /* single-byte value. */
        if (u8 & KBD_UP_BIT) {
            /* release. */
            isup = 1;
            u8 &= ~KBD_UP_BIT;
            val = keytabup[u8];
        } else {
            val = keytab1b[u8];
        }
    } else {
        /* 0xe0-prefixed. */
        kbdread(u8);
        if (u8 == KBD_PRINT_BYTE2 || u8 == KBD_CTRLPAUSE_BYTE2) {
            /* print screen or ctrl-pause. */
            kbdread(u8); /* 0xe0 */
            kbdread(u8); /* 0x37 (prtsc) or 0xc6 (ctrl-pause) */
            val &= KBD_VAL_MSK;
            val = keytabmb[u8];
        } else if (u8 == KBD_UP_BYTE) {
            kbdread(u8);
            val = keytabup[u8];
        } else {
            if (u8 & KBD_UP_BIT) {
                isup = 1;
                u8 &= ~KBD_UP_BIT;
                val = keytabup[u8];
            } else {
                val = keytabmb[u8];
            }
        }
    }
    if (ismodkey(val)) {
        if (isup) {
            modmask &= ~(1 << (-val));
        } else {
            modmask |= 1 << (-val);
        }
    }

    return;
}

void
mouseinit(void)
{
    irqvec[IRQMOUSE] = mouseintr;
    kprintf("PS/2 mouse interrupt enabled\n");

    return;
}

void
mouseintr(void)
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

void
ps2init(void)
{
    kbdinit();
    mouseinit();
}

#endif /* PS2DRV */

