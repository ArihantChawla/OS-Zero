#include <stdint.h>
#include <zero/param.h>
#include <zero/cdecl.h>
#include <sys/zero.h>
#include <sys/zero/kbd.h>

/* kbdsetkey(), kbdscan2wc() */

#define KBDMAXVAL 0x7f
#define KBDNVAL   128

#if 0
#define ismodkey(val)                                                   \
    (((val) & 0x80000000) && ((val) & 0xfffffff0) == 0xfffffff0)
#endif
#define setkeycode(name)                                                \
    (((((name) >> 8) & 0xff) == PS2KBD_UP_BYTE)                         \
     ? (keytabup[(name >> 16) & 0xff] = name##_SYM | PS2KBD_UP_BIT)     \
     : ((((name) & 0xff) == PS2KBD_PREFIX_BYTE)                         \
        ? (keytabmb[(name >> 8) & 0xff] = name##_SYM,                   \
           keytabup[(name >> 8) & 0xff] = name##_SYM | PS2KBD_UP_BIT)   \
        : (keytab1b[name & 0xff] = name##_SYM,                          \
           keytabup[name & 0xff] = name##_SYM | PS2KBD_UP_BIT)))

#if 0
/* modifier keys. */
static int32_t keytabmod[KBDNVAL];
#endif
/* single-code values. */
static int32_t keytab1b[KBDNVAL] ALIGNED(PAGESIZE);
/* 0xe0-prefixed values. */
static int32_t keytabmb[KBDNVAL];
/* release values. */
static int32_t keytabup[KBDNVAL];

void
kbdinit_us(void)
{
    /* modifiers. */
    setkeycode(PS2KBD_LEFTCTRL);
    setkeycode(PS2KBD_LEFTSHIFT);
    setkeycode(PS2KBD_RIGHTSHIFT);
    setkeycode(PS2KBD_LEFTALT);
    setkeycode(PS2KBD_RIGHTALT);
    setkeycode(PS2KBD_CAPSLOCK);
    setkeycode(PS2KBD_NUMLOCK);
    setkeycode(PS2KBD_SCROLLLOCK);

    /* single-byte keys. */

    setkeycode(PS2KBD_ESC);
    setkeycode(PS2KBD_1);
    setkeycode(PS2KBD_2);
    setkeycode(PS2KBD_3);
    setkeycode(PS2KBD_4);
    setkeycode(PS2KBD_5);
    setkeycode(PS2KBD_6);
    setkeycode(PS2KBD_7);
    setkeycode(PS2KBD_8);
    setkeycode(PS2KBD_9);
    setkeycode(PS2KBD_0);
    setkeycode(PS2KBD_MINUS);
    setkeycode(PS2KBD_PLUS);
    setkeycode(PS2KBD_BACKSPACE);

    setkeycode(PS2KBD_TAB);
    setkeycode(PS2KBD_q);
    setkeycode(PS2KBD_w);
    setkeycode(PS2KBD_e);
    setkeycode(PS2KBD_r);
    setkeycode(PS2KBD_t);
    setkeycode(PS2KBD_y);
    setkeycode(PS2KBD_u);
    setkeycode(PS2KBD_i);
    setkeycode(PS2KBD_o);
    setkeycode(PS2KBD_p);
    setkeycode(PS2KBD_OPENBRACKET);
    setkeycode(PS2KBD_CLOSEBRACKET);

    setkeycode(PS2KBD_ENTER);

    setkeycode(PS2KBD_LEFTCTRL);

    setkeycode(PS2KBD_a);
    setkeycode(PS2KBD_s);
    setkeycode(PS2KBD_d);
    setkeycode(PS2KBD_f);
    setkeycode(PS2KBD_g);
    setkeycode(PS2KBD_h);
    setkeycode(PS2KBD_i);
    setkeycode(PS2KBD_j);
    setkeycode(PS2KBD_k);
    setkeycode(PS2KBD_l);
    setkeycode(PS2KBD_SEMICOLON);
    setkeycode(PS2KBD_QUOTE);

    setkeycode(PS2KBD_BACKQUOTE);

    setkeycode(PS2KBD_LEFTSHIFT);

    setkeycode(PS2KBD_BACKSLASH);

    setkeycode(PS2KBD_z);
    setkeycode(PS2KBD_x);
    setkeycode(PS2KBD_c);
    setkeycode(PS2KBD_v);
    setkeycode(PS2KBD_b);
    setkeycode(PS2KBD_n);
    setkeycode(PS2KBD_m);
    setkeycode(PS2KBD_COMMA);
    setkeycode(PS2KBD_DOT);
    setkeycode(PS2KBD_SLASH);

    setkeycode(PS2KBD_RIGHTSHIFT);

    setkeycode(PS2KBD_KEYPADASTERISK);

    setkeycode(PS2KBD_SPACE);

    setkeycode(PS2KBD_CAPSLOCK);

    setkeycode(PS2KBD_F1);
    setkeycode(PS2KBD_F2);
    setkeycode(PS2KBD_F3);
    setkeycode(PS2KBD_F4);
    setkeycode(PS2KBD_F5);
    setkeycode(PS2KBD_F6);
    setkeycode(PS2KBD_F7);
    setkeycode(PS2KBD_F8);
    setkeycode(PS2KBD_F9);
    setkeycode(PS2KBD_F10);

    setkeycode(PS2KBD_NUMLOCK);
    setkeycode(PS2KBD_SCROLLLOCK);

    setkeycode(PS2KBD_F11);
    setkeycode(PS2KBD_F12);

    setkeycode(PS2KBD_KEYPAD7);
    setkeycode(PS2KBD_KEYPAD8);
    setkeycode(PS2KBD_KEYPAD9);

    setkeycode(PS2KBD_KEYPADMINUS2);

    setkeycode(PS2KBD_KEYPAD4);
    setkeycode(PS2KBD_KEYPAD5);
    setkeycode(PS2KBD_KEYPAD6);

    setkeycode(PS2KBD_KEYPADPLUS);

    setkeycode(PS2KBD_KEYPADEND);
    setkeycode(PS2KBD_KEYPADDOWN);
    setkeycode(PS2KBD_KEYPADPGDN);

    setkeycode(PS2KBD_KEYPADINS);
    setkeycode(PS2KBD_KEYPADDEL);

    setkeycode(PS2KBD_SYSRQ);

    /* dual-byte sequences. */

    setkeycode(PS2KBD_KEYPADENTER);
    setkeycode(PS2KBD_RIGHTCTRL);
    setkeycode(PS2KBD_FAKELEFTSHIFT);
    setkeycode(PS2KBD_KEYPADMINUS3);
    setkeycode(PS2KBD_FAKERIGHTSHIFT);
    setkeycode(PS2KBD_CTRLPRINTSCREEN);
    setkeycode(PS2KBD_RIGHTALT);
    setkeycode(PS2KBD_CTRLBREAK);
    setkeycode(PS2KBD_HOME);
    setkeycode(PS2KBD_UP);
    setkeycode(PS2KBD_PGUP);
    setkeycode(PS2KBD_LEFT);
    setkeycode(PS2KBD_RIGHT);
    setkeycode(PS2KBD_END);
    setkeycode(PS2KBD_DOWN);
    setkeycode(PS2KBD_PGDN);
    setkeycode(PS2KBD_INS);
    setkeycode(PS2KBD_DEL);

    /* acpi codes. */
    setkeycode(PS2KBD_POWER);
    setkeycode(PS2KBD_SLEEP);
    setkeycode(PS2KBD_WAKE);
    setkeycode(PS2KBD_POWERUP);
    setkeycode(PS2KBD_SLEEPUP);
    setkeycode(PS2KBD_WAKEUP);

    return;
}

